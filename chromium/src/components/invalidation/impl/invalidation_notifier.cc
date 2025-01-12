// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/invalidation/impl/invalidation_notifier.h"

#include <utility>

#include "base/bind.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "components/invalidation/public/invalidation_handler.h"
#include "google/cacheinvalidation/include/invalidation-client-factory.h"
#include "jingle/notifier/listener/push_client.h"
#include "net/url_request/url_request_context.h"

namespace syncer {

InvalidationNotifier::InvalidationNotifier(
    std::unique_ptr<SyncNetworkChannel> network_channel,
    const std::string& invalidator_client_id,
    const UnackedInvalidationsMap& saved_invalidations,
    const std::string& invalidation_bootstrap_data,
    const base::WeakPtr<InvalidationStateTracker>& invalidation_state_tracker,
    scoped_refptr<base::SingleThreadTaskRunner>
        invalidation_state_tracker_task_runner,
    const std::string& client_info)
    : state_(STOPPED),
      saved_invalidations_(saved_invalidations),
      invalidation_state_tracker_(invalidation_state_tracker),
      invalidation_state_tracker_task_runner_(
          invalidation_state_tracker_task_runner),
      client_info_(client_info),
      invalidator_client_id_(invalidator_client_id),
      invalidation_bootstrap_data_(invalidation_bootstrap_data),
      invalidation_listener_(std::move(network_channel)) {}

InvalidationNotifier::~InvalidationNotifier() {
  DCHECK(CalledOnValidThread());
}

void InvalidationNotifier::RegisterHandler(InvalidationHandler* handler) {
  DCHECK(CalledOnValidThread());
  registrar_.RegisterHandler(handler);
}

bool InvalidationNotifier::UpdateRegisteredIds(InvalidationHandler* handler,
                                               const ObjectIdSet& ids) {
  DCHECK(CalledOnValidThread());
  if (!registrar_.UpdateRegisteredIds(handler, ids))
    return false;
  invalidation_listener_.UpdateRegisteredIds(registrar_.GetAllRegisteredIds());
  return true;
}

void InvalidationNotifier::UnregisterHandler(InvalidationHandler* handler) {
  DCHECK(CalledOnValidThread());
  registrar_.UnregisterHandler(handler);
}

#if defined(OPERA_SYNC)
OperaInvalidatorState InvalidationNotifier::GetInvalidatorState() const {
#else
InvalidatorState InvalidationNotifier::GetInvalidatorState() const {
#endif  // OPERA_SYNC
  DCHECK(CalledOnValidThread());
  return registrar_.GetInvalidatorState();
}

void InvalidationNotifier::UpdateCredentials(
    const std::string& email, const std::string& token) {
  if (state_ == STOPPED) {
    invalidation_listener_.Start(
        base::Bind(&invalidation::CreateInvalidationClient),
        invalidator_client_id_,
        client_info_,
        invalidation_bootstrap_data_,
        saved_invalidations_,
        invalidation_state_tracker_,
        invalidation_state_tracker_task_runner_,
        this);
    state_ = STARTED;
  }
  invalidation_listener_.UpdateCredentials(email, token);
}

void InvalidationNotifier::RequestDetailedStatus(
    base::Callback<void(const base::DictionaryValue&)> callback) const {
  DCHECK(CalledOnValidThread());
  invalidation_listener_.RequestDetailedStatus(callback);
}

void InvalidationNotifier::OnInvalidate(
    const ObjectIdInvalidationMap& invalidation_map) {
  DCHECK(CalledOnValidThread());
  registrar_.DispatchInvalidationsToHandlers(invalidation_map);
}

#if defined(OPERA_SYNC)
void InvalidationNotifier::OnInvalidatorStateChange(
    OperaInvalidatorState state) {
#else
void InvalidationNotifier::OnInvalidatorStateChange(InvalidatorState state) {
#endif  // OPERA_SYNC
  DCHECK(CalledOnValidThread());
  registrar_.UpdateInvalidatorState(state);
}

}  // namespace syncer
