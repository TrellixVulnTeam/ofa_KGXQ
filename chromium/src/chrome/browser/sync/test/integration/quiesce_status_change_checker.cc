// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/test/integration/quiesce_status_change_checker.h"

#include <stddef.h>

#include "base/format_macros.h"
#include "base/scoped_observer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "components/browser_sync/profile_sync_service.h"
#include "components/sync/engine/cycle/sync_cycle_snapshot.h"

#ifdef OPERA_SYNC
#include <map>
#include "components/sync/base/model_type.h"
#include "components/sync/protocol/sync.pb.h"
#include "third_party/re2/src/re2/re2.h"
#endif  // OPERA_SYNC


namespace {

#ifdef OPERA_SYNC
/**
 * Opera-specific addition. This method fetches the lastest progress marker
 * for the given |type| from the given |service| and returns it as uint64.
 * In Opera server implementation the progress marker is the version of the
 * youngest entity of the given type stored on the server. The progress marker
 * has to match with the last version sent with the XMPP invalidation.
 *
 * DO NOTE however, that the method is fragile to changes in the server
 * marker generation mechanism, in that it parses the received string using
 * substr.
 */
uint64_t GetProgressMarkerForType(const ProfileSyncService* service,
                                  syncer::ModelType type) {
  std::string progress_marker =
      service->GetLastSessionSnapshot().download_progress_markers().at(type);
  sync_pb::DataTypeProgressMarker marker_message;
  CHECK(marker_message.ParseFromString(progress_marker));

  uint64_t version = 0;
  // The version is pickled in the marker, i.e. the input string looks like
  // this (the beginning):
  // (Innn\nInnn
  // We want to extract the first number (i.e. nnn) between the two "I"
  // characters. The number of digits may vary.
  CHECK(RE2::PartialMatch(marker_message.token(), "\\(L(\\d+)", &version)) <<
      "No parsable progress marker for type " << type << ". Ignoring.";

  return version;
}

bool ServiceHasLatestDirectoryUpdate(ProfileSyncService* service) {
  browser_sync::VersionMap invalidation_map =
      service->invalidation_map();

  for (auto it = invalidation_map.begin(); it != invalidation_map.end(); it++) {
    syncer::ModelType model_type = it->first;
    uint64_t progress_version = GetProgressMarkerForType(service, model_type);
    uint64_t invalidation_version = invalidation_map[model_type];

    if (progress_version > 0 && invalidation_version > progress_version)
      return false;
  }

  return true;
}
#endif  // OPERA_SYNC


// Returns true if this service is disabled.
bool IsSyncDisabled(browser_sync::ProfileSyncService* service) {
  return !service->IsSetupInProgress() && !service->IsFirstSetupComplete();
}

// Returns true if these services have matching progress markers.
bool ProgressMarkersMatch(const browser_sync::ProfileSyncService* service1,
                          const browser_sync::ProfileSyncService* service2) {
  const syncer::ModelTypeSet common_types =
      Intersection(service1->GetActiveDataTypes(),
                   service2->GetActiveDataTypes());

  const syncer::SyncCycleSnapshot& snap1 = service1->GetLastCycleSnapshot();
  const syncer::SyncCycleSnapshot& snap2 = service2->GetLastCycleSnapshot();

  for (syncer::ModelTypeSet::Iterator type_it = common_types.First();
       type_it.Good(); type_it.Inc()) {
    // Look up the progress markers.  Fail if either one is missing.
    syncer::ProgressMarkerMap::const_iterator pm_it1 =
        snap1.download_progress_markers().find(type_it.Get());
    if (pm_it1 == snap1.download_progress_markers().end()) {
      return false;
    }

    syncer::ProgressMarkerMap::const_iterator pm_it2 =
        snap2.download_progress_markers().find(type_it.Get());
    if (pm_it2 == snap2.download_progress_markers().end()) {
      return false;
    }

    // Fail if any of them don't match.
    if (pm_it1->second != pm_it2->second) {
      return false;
    }
  }
  return true;
}

}  // namespace

// A helper class to keep an eye on a particular ProfileSyncService's
// "HasLatestProgressMarkers()" state.
//
// This is a work-around for the HasLatestProgressMarkers check's inherent
// flakiness.  It's not safe to check that condition whenever we want.  The
// safest time to check it is when the ProfileSyncService emits an
// OnStateChanged() event.  This class waits for those events and updates its
// cached HasLatestProgressMarkers state every time that event occurs.
//
// See the comments in UpdatedProgressMarkerChecker for more details.
//
// The long-term plan is to deprecate this hack by replacing all its usees with
// more reliable status checkers.
class ProgressMarkerWatcher : public syncer::SyncServiceObserver {
 public:
  ProgressMarkerWatcher(browser_sync::ProfileSyncService* service,
                        QuiesceStatusChangeChecker* quiesce_checker);
  ~ProgressMarkerWatcher() override;
  void OnStateChanged() override;

  bool HasLatestProgressMarkers();
  bool IsSyncDisabled();

 private:
  void UpdateHasLatestProgressMarkers();

  browser_sync::ProfileSyncService* service_;
  QuiesceStatusChangeChecker* quiesce_checker_;
  ScopedObserver<browser_sync::ProfileSyncService, ProgressMarkerWatcher>
      scoped_observer_;
  bool probably_has_latest_progress_markers_;
};

ProgressMarkerWatcher::ProgressMarkerWatcher(
    browser_sync::ProfileSyncService* service,
    QuiesceStatusChangeChecker* quiesce_checker)
    : service_(service),
      quiesce_checker_(quiesce_checker),
      scoped_observer_(this),
      probably_has_latest_progress_markers_(false) {
  scoped_observer_.Add(service);
  UpdateHasLatestProgressMarkers();
}

ProgressMarkerWatcher::~ProgressMarkerWatcher() { }

void ProgressMarkerWatcher::OnStateChanged() {
  UpdateHasLatestProgressMarkers();
  quiesce_checker_->OnServiceStateChanged(service_);
}

void ProgressMarkerWatcher::UpdateHasLatestProgressMarkers() {
  if (IsSyncDisabled()) {
    probably_has_latest_progress_markers_ = false;
    return;
  }

  // This is the same progress marker check as used by the
  // UpdatedProgressMarkerChecker.  It has the samed drawbacks and potential for
  // flakiness.  See the comment in
  // UpdatedProgressMarkerChecker::IsExitConditionSatisfied() for more
  // information.
  //
  // The QuiesceStatusChangeChecker attempts to work around the limitations of
  // this progress marker checking method.  It tries to update the progress
  // marker status only in the OnStateChanged() callback, where the snapshot is
  // freshest.
  //
  // It also checks the progress marker status when it is first initialized, and
  // that's where it's most likely that we could return a false positive.  We
  // need to check these service at startup, since not every service is
  // guaranteed to generate OnStateChanged() events while we're waiting for
  // quiescence.
  const syncer::SyncCycleSnapshot& snap = service_->GetLastCycleSnapshot();
  probably_has_latest_progress_markers_ =
      snap.model_neutral_state().num_successful_commits == 0 &&
      !service_->HasUnsyncedItems();
}

bool ProgressMarkerWatcher::HasLatestProgressMarkers() {
  return probably_has_latest_progress_markers_;
}

bool ProgressMarkerWatcher::IsSyncDisabled() {
  return ::IsSyncDisabled(service_);
}

QuiesceStatusChangeChecker::QuiesceStatusChangeChecker(
    std::vector<browser_sync::ProfileSyncService*> services)
    : services_(services) {
  DCHECK_LE(1U, services_.size());
  for (size_t i = 0; i < services_.size(); ++i) {
    observers_.push_back(new ProgressMarkerWatcher(services[i], this));
  }
}

QuiesceStatusChangeChecker::~QuiesceStatusChangeChecker() {}

bool QuiesceStatusChangeChecker::IsExitConditionSatisfied() {
  // Check that all progress markers are up to date.
  for (ScopedVector<ProgressMarkerWatcher>::const_iterator it =
       observers_.begin(); it != observers_.end(); ++it) {
    if ((*it)->IsSyncDisabled()) {
      continue;  // Skip disabled services.
    }

    if (!(*it)->HasLatestProgressMarkers()) {
      VLOG(1) << "Not quiesced: Progress markers are old.";
      return false;
    }
  }

  std::vector<browser_sync::ProfileSyncService*> enabled_services;
  for (std::vector<browser_sync::ProfileSyncService*>::const_iterator it =
           services_.begin();
       it != services_.end(); ++it) {
    if (!IsSyncDisabled(*it)) {
      enabled_services.push_back(*it);
    }
  }

  // Return true if we have nothing to compare against.
  if (enabled_services.size() <= 1) {
    return true;
  }

#ifdef OPERA_SYNC
  for (auto it = enabled_services.begin(); it != enabled_services.end(); it++) {
    if (!ServiceHasLatestDirectoryUpdate(*it)) {
      VLOG(1) << "Not quiesced: Invalidation informs about a newer version "
                 "than we have.";
      return false;
    }
  }
#endif  // OPERA_SYNC

  std::vector<browser_sync::ProfileSyncService*>::const_iterator it1 =
      enabled_services.begin();
  std::vector<browser_sync::ProfileSyncService*>::const_iterator it2 =
      enabled_services.begin();
  it2++;

  while (it2 != enabled_services.end()) {
    // Return false if there is a progress marker mismatch.
    if (!ProgressMarkersMatch(*it1, *it2)) {
      VLOG(1) << "Not quiesced: Progress marker mismatch.";
      return false;
    }
    it1++;
    it2++;
  }

  return true;
}

std::string QuiesceStatusChangeChecker::GetDebugMessage() const {
  return base::StringPrintf("Waiting for quiescence of %" PRIuS " clients",
                            services_.size());
}

void QuiesceStatusChangeChecker::OnServiceStateChanged(
    browser_sync::ProfileSyncService* service) {
  CheckExitCondition();
}
