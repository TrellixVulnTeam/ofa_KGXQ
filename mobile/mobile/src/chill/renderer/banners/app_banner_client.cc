// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/renderer/banners/app_banner_client.cc
// @final-synchronized

#include "chill/renderer/banners/app_banner_client.h"

#include "ipc/ipc_message.h"
#include "third_party/WebKit/public/platform/WebString.h"

#include "chill/common/app_banner_messages.h"

using blink::WebString;

AppBannerClient::AppBannerClient(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {
}

AppBannerClient::~AppBannerClient() {
}

void AppBannerClient::OnDestruct() {}

bool AppBannerClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(AppBannerClient, message)
    IPC_MESSAGE_HANDLER(OperaViewMsg_AppBannerAccepted, OnBannerAccepted);
    IPC_MESSAGE_HANDLER(OperaViewMsg_AppBannerDismissed, OnBannerDismissed);
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void AppBannerClient::registerBannerCallbacks(
    int request_id,
    blink::WebAppBannerCallbacks* callbacks) {
  banner_callbacks_.AddWithID(callbacks, request_id);
}

void AppBannerClient::showAppBanner(int request_id) {
  Send(new OperaViewHostMsg_RequestShowAppBanner(routing_id(), request_id));
}

void AppBannerClient::ResolveEvent(
    int request_id,
    const std::string& platform,
    const blink::WebAppBannerPromptResult::Outcome& outcome) {
  blink::WebAppBannerCallbacks* callbacks =
      banner_callbacks_.Lookup(request_id);
  if (!callbacks)
    return;

  callbacks->onSuccess(blink::WebAppBannerPromptResult(
      blink::WebString::fromUTF8(platform), outcome));
  banner_callbacks_.Remove(request_id);
}

void AppBannerClient::OnBannerAccepted(int request_id,
                                       const std::string& platform) {
  ResolveEvent(request_id, platform,
               blink::WebAppBannerPromptResult::Outcome::Accepted);
}

void AppBannerClient::OnBannerDismissed(int request_id) {
  ResolveEvent(request_id, "",
               blink::WebAppBannerPromptResult::Outcome::Dismissed);
}
