// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software AS
// @copied-from chromium/src/browser/push_messaging/push_messaging_app_identifier.h
// @final-synchronized

#ifndef CHILL_BROWSER_PUSH_MESSAGING_PUSH_MESSAGING_APP_IDENTIFIER_H_
#define CHILL_BROWSER_PUSH_MESSAGING_PUSH_MESSAGING_APP_IDENTIFIER_H_

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/logging.h"
#include "url/gurl.h"

// The prefix used for all push messaging application ids.
extern const char kPushMessagingAppIdentifierPrefix[];

// Forward-declare Profile (even though it's unused), to reduce differences
// with upstream.
class Profile;

namespace opera {

// Type used to identify a Service Worker registration from a Push API
// perspective. These can be persisted to prefs, in a 1:1 mapping between
// app_id (which includes origin) and service_worker_registration_id.
// Legacy mapped values saved by old versions of Chrome are also supported;
// these don't contain the origin in the app_id, so instead they map from
// app_id to pair<origin, service_worker_registration_id>.
class PushMessagingAppIdentifier {
 public:
  // Generates a new app identifier, with partially random app_id.
  static PushMessagingAppIdentifier Generate(
      const GURL& origin,
      int64_t service_worker_registration_id);

  // Looks up an app identifier by app_id. If not found, is_null() will be true.
  static PushMessagingAppIdentifier FindByAppId(
      Profile* profile,
      const std::string& app_id);

  // Looks up an app identifier by origin & service worker registration id.
  // If not found, is_null() will be true.
  static PushMessagingAppIdentifier FindByServiceWorker(
      Profile* profile,
      const GURL& origin,
      int64_t service_worker_registration_id);

  // Returns all the PushMessagingAppIdentifiers currently registered for the
  // given |profile|.
  static std::vector<PushMessagingAppIdentifier> GetAll(
      Profile* profile);

  // Returns the number of PushMessagingAppIdentifiers currently registered for
  // the given |profile|.
  static size_t GetCount(Profile* profile);

  ~PushMessagingAppIdentifier();

  // Persist this app identifier to prefs.
  void PersistToPrefs(Profile* profile) const;

  // Delete this app identifier from prefs.
  void DeleteFromPrefs(Profile* profile) const;

  // Returns true if this identifier does not represent an app (i.e. this was
  // returned by a failed Find call).
  bool is_null() const { return service_worker_registration_id_ < 0; }

  // String that should be passed to push services like GCM to identify a
  // particular Service Worker (so we can route incoming messages). Example:
  // wp:https://foo.example.com:8443/#9CC55CCE-B8F9-4092-A364-3B0F73A3AB5F
  // Legacy app_ids have no origin, e.g. wp:9CC55CCE-B8F9-4092-A364-3B0F73A3AB5F
  const std::string& app_id() const {
    DCHECK(!is_null());
    return app_id_;
  }

  const GURL& origin() const {
    DCHECK(!is_null());
    return origin_;
  }

  int64_t service_worker_registration_id() const {
    DCHECK(!is_null());
    return service_worker_registration_id_;
  }

 private:
  // Constructs an invalid app identifier.
  PushMessagingAppIdentifier();
  // Constructs a valid app identifier.
  PushMessagingAppIdentifier(const std::string& app_id,
                             const GURL& origin,
                             int64_t service_worker_registration_id);

  // Validates that all the fields contain valid values.
  void DCheckValid() const;

  std::string app_id_;
  GURL origin_;
  int64_t service_worker_registration_id_;
};

}  // namespace opera

#endif  // CHILL_BROWSER_PUSH_MESSAGING_PUSH_MESSAGING_APP_IDENTIFIER_H_
