// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/browser/history/history_tab_helper.cc
// @final-synchronized

#ifndef CHILL_BROWSER_HISTORY_TAB_HELPER_H_
#define CHILL_BROWSER_HISTORY_TAB_HELPER_H_

#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace base {
class Time;
}

namespace history {
struct HistoryAddPageArgs;
class HistoryService;
}

class Profile;

// Observes WebContents and notifies HistoryService of navigations.
class HistoryTabHelper : public content::WebContentsObserver,
                         public content::WebContentsUserData<HistoryTabHelper> {
 public:
  ~HistoryTabHelper() override;

 private:
  // Updates history with the specified navigation. This is called by
  // OnMsgNavigate to update history state.
  void UpdateHistoryForNavigation(
      const history::HistoryAddPageArgs& add_page_args);

  // Sends the page title to the history service. This is called when we receive
  // the page title and we know we want to update history.
  void UpdateHistoryPageTitle(const content::NavigationEntry& entry);

  // Returns the history::HistoryAddPageArgs to use for adding a page to
  // history.
  history::HistoryAddPageArgs CreateHistoryAddPageArgs(
      const GURL& virtual_url,
      base::Time timestamp,
      bool did_replace_entry,
      int nav_entry_id,
      const content::FrameNavigateParams& params);

  explicit HistoryTabHelper(content::WebContents* web_contents);
  friend class content::WebContentsUserData<HistoryTabHelper>;

  // content::WebContentsObserver implementation.
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  void DidNavigateAnyFrame(content::RenderFrameHost* render_frame_host,
                           const content::LoadCommittedDetails& details,
                           const content::FrameNavigateParams& params) override;
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) override;
  void WebContentsDestroyed() override;

  // Helper function to return the history service.  May return NULL.
  history::HistoryService* GetHistoryService();

  // Whether we have a (non-empty) title for the current page.
  // Used to prevent subsequent title updates from affecting history. This
  // prevents some weirdness because some AJAXy apps use titles for status
  // messages.
  bool received_page_title_;
  Profile* profile_;

  DISALLOW_COPY_AND_ASSIGN(HistoryTabHelper);
};

#endif  // CHILL_BROWSER_HISTORY_TAB_HELPER_H_
