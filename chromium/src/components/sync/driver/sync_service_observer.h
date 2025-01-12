// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_SYNC_SERVICE_OBSERVER_H_
#define COMPONENTS_SYNC_DRIVER_SYNC_SERVICE_OBSERVER_H_

#if defined(OPERA_SYNC)
#include "components/invalidation/public/invalidator_state.h"
#endif  // OPERA_SYNC

namespace syncer {

// Various UI components such as the New Tab page can be driven by observing
// the SyncService through this interface.
class SyncServiceObserver {
 public:
  // When one of the following events occurs, OnStateChanged() is called.
  // Observers should query the service to determine what happened.
  // - We initialized successfully.
  // - The sync servers are unavailable at this time.
  // - Credentials are now in flight for authentication.
  // - The data type configuration has started or ended.
  // - Sync shut down.
  // - Sync errors (passphrase, auth, unrecoverable, actionable, etc.).
  // - Encryption changes.
  virtual void OnStateChanged() = 0;

  // If a client wishes to handle sync cycle completed events in a special way,
  // they can use this function.  By default, it re-routes to OnStateChanged().
  virtual void OnSyncCycleCompleted();

#if defined(OPERA_SYNC)
  // Called when the invalidator state changes, allows tracking the
  // enabled/disabled/transient error state along with the last auth
  // problem, if available.
  virtual void OnInvalidatorStateChanged(syncer::OperaInvalidatorState state);
#endif  // OPERA_SYNC

  // Called when the sync service has finished the datatype configuration
  // process.
  virtual void OnSyncConfigurationCompleted() {}

  // Called when a foreign session has been updated.
  virtual void OnForeignSessionUpdated() {}

 protected:
  virtual ~SyncServiceObserver() {}
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_DRIVER_SYNC_SERVICE_OBSERVER_H_
