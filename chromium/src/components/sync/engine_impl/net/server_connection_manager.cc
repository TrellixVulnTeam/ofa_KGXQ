// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine_impl/net/server_connection_manager.h"

#include <errno.h>

#include <ostream>
#include <vector>

#include "base/metrics/histogram.h"
#include "build/build_config.h"
#include "components/sync/base/cancelation_signal.h"
#include "components/sync/engine_impl/net/url_translator.h"
#include "components/sync/engine_impl/syncer.h"
#include "components/sync/protocol/sync.pb.h"
#include "components/sync/syncable/directory.h"
#include "net/base/net_errors.h"
#include "net/http/http_status_code.h"
#include "url/gurl.h"

namespace syncer {

using std::ostream;
using std::string;
using std::vector;

static const char kSyncServerSyncPath[] = "/command/";

HttpResponse::HttpResponse()
    : response_code(kUnsetResponseCode),
      content_length(kUnsetContentLength),
      payload_length(kUnsetPayloadLength),
      server_status(NONE) {}

#define ENUM_CASE(x) \
  case x:            \
    return #x;       \
    break

const char* HttpResponse::GetServerConnectionCodeString(
    ServerConnectionCode code) {
  switch (code) {
    ENUM_CASE(NONE);
    ENUM_CASE(CONNECTION_UNAVAILABLE);
    ENUM_CASE(IO_ERROR);
    ENUM_CASE(SYNC_SERVER_ERROR);
    ENUM_CASE(SYNC_AUTH_ERROR);
    ENUM_CASE(SERVER_CONNECTION_OK);
#if defined(OPERA_SYNC)
    ENUM_CASE(LOAD_BALANCER_ERROR);
#endif
    ENUM_CASE(RETRY);
  }
  NOTREACHED();
  return "";
}

#undef ENUM_CASE

ServerConnectionManager::Connection::Connection(ServerConnectionManager* scm)
    : scm_(scm) {}

ServerConnectionManager::Connection::~Connection() {}

bool ServerConnectionManager::Connection::ReadBufferResponse(
    string* buffer_out,
    HttpResponse* response,
    bool require_response) {
  if (net::HTTP_OK != response->response_code) {
    response->server_status = HttpResponse::SYNC_SERVER_ERROR;
    return false;
  }

  if (require_response && (1 > response->content_length))
    return false;

  const int64_t bytes_read =
      ReadResponse(buffer_out, static_cast<int>(response->content_length));
  if (bytes_read != response->content_length) {
    response->server_status = HttpResponse::IO_ERROR;
    return false;
  }
  return true;
}

bool ServerConnectionManager::Connection::ReadDownloadResponse(
    HttpResponse* response,
    string* buffer_out) {
  const int64_t bytes_read =
      ReadResponse(buffer_out, static_cast<int>(response->content_length));

  if (bytes_read != response->content_length) {
    LOG(ERROR) << "Mismatched content lengths, server claimed "
               << response->content_length << ", but sent " << bytes_read;
    response->server_status = HttpResponse::IO_ERROR;
    return false;
  }
  return true;
}

ServerConnectionManager::ScopedConnectionHelper::ScopedConnectionHelper(
    ServerConnectionManager* manager,
    Connection* connection)
    : manager_(manager), connection_(connection) {}

ServerConnectionManager::ScopedConnectionHelper::~ScopedConnectionHelper() {
  if (connection_)
    manager_->OnConnectionDestroyed(connection_.get());
  connection_.reset();
}

ServerConnectionManager::Connection*
ServerConnectionManager::ScopedConnectionHelper::get() {
  return connection_.get();
}

namespace {

string StripTrailingSlash(const string& s) {
  int stripped_end_pos = s.size();
  if (s.at(stripped_end_pos - 1) == '/') {
    stripped_end_pos = stripped_end_pos - 1;
  }

  return s.substr(0, stripped_end_pos);
}

}  // namespace

// TODO(chron): Use a GURL instead of string concatenation.
string ServerConnectionManager::Connection::MakeConnectionURL(
    const string& sync_server,
    const string& path,
    bool use_ssl) const {
  string connection_url = (use_ssl ? "https://" : "http://");
  connection_url += sync_server;
  connection_url = StripTrailingSlash(connection_url);
  connection_url += path;

  return connection_url;
}

int ServerConnectionManager::Connection::ReadResponse(string* out_buffer,
                                                      int length) {
  int bytes_read = buffer_.length();
  CHECK(length <= bytes_read);
  out_buffer->assign(buffer_);
  return bytes_read;
}

ServerConnectionManager::ServerConnectionManager(
    const string& server,
    int port,
    bool use_ssl,
    CancelationSignal* cancelation_signal
#if defined(OPERA_SYNC)
    ,
    const sync_oauth_signer::SignParams& sign_params
#endif
    )
    : sync_server_(server),
      sync_server_port_(port),
      use_ssl_(use_ssl),
      proto_sync_path_(kSyncServerSyncPath),
      server_status_(HttpResponse::NONE),
      terminated_(false),
      active_connection_(NULL),
      cancelation_signal_(cancelation_signal),
      signal_handler_registered_(false)
#if defined(OPERA_SYNC)
      ,
      oauth_sign_params_(sign_params)
#endif
{
  signal_handler_registered_ = cancelation_signal_->TryRegisterHandler(this);
  if (!signal_handler_registered_) {
    // Calling a virtual function from a constructor.  We can get away with it
    // here because ServerConnectionManager::OnSignalReceived() is the function
    // we want to call.
    OnSignalReceived();
  }
}

ServerConnectionManager::~ServerConnectionManager() {
  if (signal_handler_registered_) {
    cancelation_signal_->UnregisterHandler(this);
  }
}

ServerConnectionManager::Connection*
ServerConnectionManager::MakeActiveConnection() {
  base::AutoLock lock(terminate_connection_lock_);
  DCHECK(!active_connection_);
  if (terminated_)
    return NULL;

  active_connection_ = MakeConnection();
  return active_connection_;
}

void ServerConnectionManager::OnConnectionDestroyed(Connection* connection) {
  DCHECK(connection);
  base::AutoLock lock(terminate_connection_lock_);
  // |active_connection_| can be NULL already if it was aborted. Also,
  // it can legitimately be a different Connection object if a new Connection
  // was created after a previous one was Aborted and destroyed.
  if (active_connection_ != connection)
    return;

  active_connection_ = NULL;
}

bool ServerConnectionManager::SetAuthToken(const std::string& auth_token,
                                           const std::string& auth_secret,
                                           int64_t auth_time_skew) {
  DCHECK(thread_checker_.CalledOnValidThread());
  // With the current implementation of the token expiration signal
  // being sent both from the sync and push clients (i.e. the calls
  // to AuthDataExpired()) it is possible that we're getting the same
  // token in response. We want the sync engine to move on in such
  // a case. In particular we don't want the sync engine to generate
  // an artificial auth error.
#ifndef OPERA_SYNC
  if (previously_invalidated_token != auth_token) {
#endif  // OPERA_SYNC
    // Remember that UpdateTimeSkew needs to fit into the condition above
    // should the condition be restored for any reason. We'll be hitting a
    // nasty bug of not updating the sign params and/or generating the
    // artificial SYNC_AUTH_ERROR state in case timeskew is updated and the
    // token is not.
    // The condition should be if(UpdateTimeSkew(auth_time_skew) || p != a).
#if defined(OPERA_SYNC)
    oauth_sign_params_.UpdateTimeSkew(auth_time_skew);
#endif  // OPERA_SYNC
    auth_token_.assign(auth_token);
#if defined(OPERA_SYNC)
    oauth_sign_params_.UpdateToken(auth_token, auth_secret);
#endif  // OPERA_SYNC
    previously_invalidated_token = std::string();
    return true;
#ifndef OPERA_SYNC
  }

  // This could happen in case like server outage/bug. E.g. token returned by
  // first request is considered invalid by sync server and because
  // of token server's caching policy, etc, same token is returned on second
  // request. Need to notify sync frontend again to request new token,
  // otherwise backend will stay in SYNC_AUTH_ERROR state while frontend thinks
  // everything is fine and takes no actions.
  SetServerStatus(HttpResponse::SYNC_AUTH_ERROR);
  return false;
#endif  // OPERA_SYNC
}

void ServerConnectionManager::InvalidateAndClearAuthToken() {
  DCHECK(thread_checker_.CalledOnValidThread());
  // Copy over the token to previous invalid token.
  if (!auth_token_.empty()) {
    previously_invalidated_token.assign(auth_token_);
    auth_token_ = std::string();
#if defined(OPERA_SYNC)
    oauth_sign_params_.ClearToken();
#endif
  }
}

void ServerConnectionManager::SetServerStatus(
    HttpResponse::ServerConnectionCode server_status) {
  // SYNC_AUTH_ERROR is permanent error. Need to notify observer to take
  // action externally to resolve.
  if (server_status != HttpResponse::SYNC_AUTH_ERROR &&
      server_status_ == server_status) {
    return;
  }
  server_status_ = server_status;
  NotifyStatusChanged();
}

#if defined(OPERA_SYNC)
void ServerConnectionManager::SetAuthProblem(
    opera_sync::OperaAuthProblem problem) {
  server_auth_problem_ = problem;
}
#endif  // OPERA_SYNC

void ServerConnectionManager::NotifyStatusChanged() {
  DCHECK(thread_checker_.CalledOnValidThread());
#if defined(OPERA_SYNC)
  FOR_EACH_OBSERVER(ServerConnectionEventListener, listeners_,
                    OnServerConnectionEvent(ServerConnectionEvent(
                        server_status_, server_auth_problem_)));
#else
  FOR_EACH_OBSERVER(
      ServerConnectionEventListener, listeners_,
      OnServerConnectionEvent(ServerConnectionEvent(server_status_)));
#endif  // OPERA_SYNC
}

bool ServerConnectionManager::PostBufferWithCachedAuth(
    PostBufferParams* params) {
  DCHECK(thread_checker_.CalledOnValidThread());
  std::string path =
        MakeSyncServerPath(proto_sync_path(), MakeSyncQueryString(client_id_));
  bool result = PostBufferToPath(params, path, auth_token());
#if defined(OPERA_SYNC)
  SetAuthProblem(params->response.auth_problem);
#endif  // OPERA_SYNC
  SetServerStatus(params->response.server_status);
  return result;
}

bool ServerConnectionManager::PostBufferToPath(PostBufferParams* params,
                                               const string& path,
                                               const string& auth_token) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // TODO(pavely): crbug.com/273096. Check for "credentials_lost" is added as
  // workaround for M29 blocker to avoid sending RPC to sync with known invalid
  // token but instead to trigger refreshing token in ProfileSyncService. Need
  // to clean it.
  if (auth_token.empty() || auth_token == "credentials_lost") {
    params->response.server_status = HttpResponse::SYNC_AUTH_ERROR;
    // Print a log to distinguish this "known failure" from others.
    LOG(WARNING) << "ServerConnectionManager forcing SYNC_AUTH_ERROR";
    return false;
  }

  // When our connection object falls out of scope, it clears itself from
  // active_connection_.
  ScopedConnectionHelper post(this, MakeActiveConnection());
  if (!post.get()) {
    params->response.server_status = HttpResponse::CONNECTION_UNAVAILABLE;
    return false;
  }

  VLOG(1) << "Request signed with " << auth_token;
  // Note that |post| may be aborted by now, which will just cause Init to fail
  // with CONNECTION_UNAVAILABLE.
  bool ok = post.get()->Init(path.c_str(), auth_token, params->buffer_in,
                             &params->response
#if defined(OPERA_SYNC)
                             , oauth_sign_params_
#endif
);

  if (params->response.server_status == HttpResponse::SYNC_AUTH_ERROR) {
    VLOG(1) << "Authorization error";
    InvalidateAndClearAuthToken();
  }
  if (!ok || net::HTTP_OK != params->response.response_code) {
    VLOG(1) << "Result: " << (ok ? "OK" : "Error") <<
        " (" << params->response.response_code << ")";
    return false;
  }

  if (post.get()->ReadBufferResponse(&params->buffer_out, &params->response,
                                     true)) {
    params->response.server_status = HttpResponse::SERVER_CONNECTION_OK;
    return true;
  }

  return false;
}

void ServerConnectionManager::AddListener(
    ServerConnectionEventListener* listener) {
  DCHECK(thread_checker_.CalledOnValidThread());
  listeners_.AddObserver(listener);
}

void ServerConnectionManager::RemoveListener(
    ServerConnectionEventListener* listener) {
  DCHECK(thread_checker_.CalledOnValidThread());
  listeners_.RemoveObserver(listener);
}

ServerConnectionManager::Connection* ServerConnectionManager::MakeConnection() {
  return NULL;  // For testing.
}

void ServerConnectionManager::OnSignalReceived() {
  base::AutoLock lock(terminate_connection_lock_);
  terminated_ = true;
  if (active_connection_)
    active_connection_->Abort();

  // Sever our ties to this connection object. Note that it still may exist,
  // since we don't own it, but it has been neutered.
  active_connection_ = NULL;
}

std::ostream& operator<<(std::ostream& s, const struct HttpResponse& hr) {
  s << " Response Code (bogus on error): " << hr.response_code;
  s << " Content-Length (bogus on error): " << hr.content_length;
  s << " Server Status: "
    << HttpResponse::GetServerConnectionCodeString(hr.server_status);
  return s;
}

}  // namespace syncer
