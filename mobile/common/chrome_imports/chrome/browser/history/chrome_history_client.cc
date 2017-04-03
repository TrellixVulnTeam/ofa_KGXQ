// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/history/chrome_history_client.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "chrome/browser/history/chrome_history_backend_client.h"
#include "chrome/browser/history/history_utils.h"
#include "chrome/browser/ui/profile_error_dialog.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/history/core/browser/history_service.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/url_constants.h"
#include "url/gurl.h"

#if defined(OS_ANDROID)
namespace {
// We should not show operaui links in suggestions
const char kOperaInternalUIScheme[] = "operaui";

bool IsValidURL(const GURL& url) {
  if (!url.is_valid())
    return false;
  if (url.SchemeIs(url::kJavaScriptScheme) ||
#if !defined(OPERA_MINI)
      url.SchemeIs(content::kChromeDevToolsScheme) ||
#endif
      url.SchemeIs(content::kChromeUIScheme) ||
#if !defined(OPERA_MINI)
      url.SchemeIs(content::kViewSourceScheme) ||
#endif
      url.SchemeIs(kOperaInternalUIScheme))
    return false;

  // Allow all about: and chrome: URLs except about:blank, since the user may
  // like to see "chrome://memory/", etc. in their history and autocomplete.
  if (url == GURL(url::kAboutBlankURL))
    return false;

  return true;
}

}  // namespace
#endif

#if defined(OS_IOS)
namespace {

bool IsValidURL(const GURL& url) {
  if (!url.is_valid())
    return false;
  if (url.SchemeIs(url::kJavaScriptScheme) ||
      url.SchemeIs(content::kChromeUIScheme))
    return false;

  // Allow all about: and chrome: URLs except about:blank, since the user may
  // like to see "chrome://memory/", etc. in their history and autocomplete.
  if (url == GURL(url::kAboutBlankURL))
    return false;

  return true;
}

}  // namespace
#endif

ChromeHistoryClient::ChromeHistoryClient(
    bookmarks::BookmarkModel* bookmark_model)
    : bookmark_model_(bookmark_model), is_bookmark_model_observer_(false) {
}

ChromeHistoryClient::~ChromeHistoryClient() {
}

void ChromeHistoryClient::OnHistoryServiceCreated(
    history::HistoryService* history_service) {
  DCHECK(!is_bookmark_model_observer_);
  if (bookmark_model_) {
    on_bookmarks_removed_ =
        base::Bind(&history::HistoryService::URLsNoLongerBookmarked,
                   base::Unretained(history_service));
    favicons_changed_subscription_ =
        history_service->AddFaviconsChangedCallback(
            base::Bind(&bookmarks::BookmarkModel::OnFaviconsChanged,
                       base::Unretained(bookmark_model_)));
    bookmark_model_->AddObserver(this);
    is_bookmark_model_observer_ = true;
  }
}

void ChromeHistoryClient::Shutdown() {
  // It's possible that bookmarks haven't loaded and history is waiting for
  // bookmarks to complete loading. In such a situation history can't shutdown
  // (meaning if we invoked HistoryService::Cleanup now, we would deadlock). To
  // break the deadlock we tell BookmarkModel it's about to be deleted so that
  // it can release the signal history is waiting on, allowing history to
  // shutdown (HistoryService::Cleanup to complete). In such a scenario history
  // sees an incorrect view of bookmarks, but it's better than a deadlock.
  if (bookmark_model_) {
    if (is_bookmark_model_observer_) {
      is_bookmark_model_observer_ = false;
      bookmark_model_->RemoveObserver(this);
      favicons_changed_subscription_.reset();
      on_bookmarks_removed_.Reset();
    }
    bookmark_model_->Shutdown();
  }
}

bool ChromeHistoryClient::CanAddURL(const GURL& url) {
  return IsValidURL(url);
}

void ChromeHistoryClient::NotifyProfileError(sql::InitStatus init_status,
                                             const std::string& diagnostics) {
}

std::unique_ptr<history::HistoryBackendClient>
ChromeHistoryClient::CreateBackendClient() {
  return base::WrapUnique(new ChromeHistoryBackendClient(bookmark_model_));
}

void ChromeHistoryClient::BookmarkModelChanged() {
}

void ChromeHistoryClient::BookmarkNodeRemoved(
    bookmarks::BookmarkModel* bookmark_model,
    const bookmarks::BookmarkNode* parent,
    int old_index,
    const bookmarks::BookmarkNode* node,
    const std::set<GURL>& removed_urls) {
  BaseBookmarkModelObserver::BookmarkNodeRemoved(bookmark_model, parent,
                                                 old_index, node, removed_urls);
  DCHECK(!on_bookmarks_removed_.is_null());
  on_bookmarks_removed_.Run(removed_urls);
}

void ChromeHistoryClient::BookmarkAllUserNodesRemoved(
    bookmarks::BookmarkModel* bookmark_model,
    const std::set<GURL>& removed_urls) {
  BaseBookmarkModelObserver::BookmarkAllUserNodesRemoved(bookmark_model,
                                                         removed_urls);
  DCHECK(!on_bookmarks_removed_.is_null());
  on_bookmarks_removed_.Run(removed_urls);
}