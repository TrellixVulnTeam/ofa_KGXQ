// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/browser/history/history_tab_helper.cc
// @final-synchronized

#include "chill/browser/history_tab_helper.h"

#include <string>
#include <utility>

#include "base/time/time.h"

// These are included through mobile/common/sync/sync.gyp.
#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/profile.h"

#include "components/history/content/browser/history_context_helper.h"
#include "components/history/core/browser/history_service.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/frame_navigate_params.h"

#include "common/sync/sync.h"

using content::NavigationEntry;
using content::WebContents;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(HistoryTabHelper);

HistoryTabHelper::HistoryTabHelper(WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      received_page_title_(false),
      profile_(mobile::Sync::GetInstance()->GetProfile()) {
}

HistoryTabHelper::~HistoryTabHelper() {
}

void HistoryTabHelper::UpdateHistoryForNavigation(
    const history::HistoryAddPageArgs& add_page_args) {
  history::HistoryService* hs = GetHistoryService();
  if (hs)
    hs->AddPage(add_page_args);
}

void HistoryTabHelper::UpdateHistoryPageTitle(const NavigationEntry& entry) {
  history::HistoryService* hs = GetHistoryService();
  if (hs)
    hs->SetPageTitle(entry.GetVirtualURL(), entry.GetTitleForDisplay());
}

history::HistoryAddPageArgs
HistoryTabHelper::CreateHistoryAddPageArgs(
    const GURL& virtual_url,
    base::Time timestamp,
    bool did_replace_entry,
    int nav_entry_id,
    const content::FrameNavigateParams& params) {
  history::HistoryAddPageArgs add_page_args(
      params.url, timestamp, history::ContextIDForWebContents(web_contents()),
      nav_entry_id, params.referrer.url, params.redirects, params.transition,
      history::SOURCE_BROWSED, did_replace_entry, false);
  if (ui::PageTransitionIsMainFrame(params.transition) &&
      virtual_url != params.url) {
    // Hack on the "virtual" URL so that it will appear in history. For some
    // types of URLs, we will display a magic URL that is different from where
    // the page is actually navigated. We want the user to see in history what
    // they saw in the URL bar, so we add the virtual URL as a redirect.  This
    // only applies to the main frame, as the virtual URL doesn't apply to
    // sub-frames.
    add_page_args.url = virtual_url;
    if (!add_page_args.redirects.empty())
      add_page_args.redirects.back() = virtual_url;
  }
  return add_page_args;
}

void HistoryTabHelper::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  // Allow the new page to set the title again.
  received_page_title_ = false;
}

void HistoryTabHelper::DidNavigateAnyFrame(
    content::RenderFrameHost* render_frame_host,
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  // Update history. Note that this needs to happen after the entry is complete,
  // which WillNavigate[Main,Sub]Frame will do before this function is called.
  if (!params.should_update_history)
    return;

  // Most of the time, the displayURL matches the loaded URL, but for about:
  // URLs, we use a data: URL as the real value.  We actually want to save the
  // about: URL to the history db and keep the data: URL hidden. This is what
  // the WebContents' URL getter does.
  NavigationEntry* last_committed =
      web_contents()->GetController().GetLastCommittedEntry();
  const history::HistoryAddPageArgs& add_page_args =
      CreateHistoryAddPageArgs(
          web_contents()->GetURL(), details.entry->GetTimestamp(),
          details.did_replace_entry, last_committed->GetUniqueID(), params);

  UpdateHistoryForNavigation(add_page_args);
}

void HistoryTabHelper::TitleWasSet(NavigationEntry* entry, bool explicit_set) {
  if (received_page_title_)
    return;

  if (entry) {
    UpdateHistoryPageTitle(*entry);
    received_page_title_ = explicit_set;
  }
}

history::HistoryService* HistoryTabHelper::GetHistoryService() {
  return HistoryServiceFactory::GetForProfile(
      profile_, ServiceAccessType::IMPLICIT_ACCESS);
}

void HistoryTabHelper::WebContentsDestroyed() {
  // We update the history for this URL.
  WebContents* tab = web_contents();

  history::HistoryService* hs = HistoryServiceFactory::GetForProfile(
      profile_, ServiceAccessType::IMPLICIT_ACCESS);
  if (hs) {
    NavigationEntry* entry = tab->GetController().GetLastCommittedEntry();
    history::ContextID context_id = history::ContextIDForWebContents(tab);
    if (entry) {
      hs->UpdateWithPageEndTime(context_id, entry->GetUniqueID(), tab->GetURL(),
                                base::Time::Now());
    }
    hs->ClearCachedDataForContextID(context_id);
  }
}
