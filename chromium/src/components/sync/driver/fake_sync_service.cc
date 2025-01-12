// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/driver/fake_sync_service.h"

#include "base/memory/ptr_util.h"
#include "base/values.h"
#include "components/sync/core/base_transaction.h"
#include "components/sync/core/user_share.h"
#include "components/sync/driver/data_type_controller.h"

namespace syncer {

FakeSyncService::FakeSyncService()
    : error_(GoogleServiceAuthError::NONE),
      user_share_(base::MakeUnique<UserShare>()) {}

FakeSyncService::~FakeSyncService() {}

bool FakeSyncService::IsFirstSetupComplete() const {
  return false;
}

bool FakeSyncService::IsSyncAllowed() const {
  return false;
}

bool FakeSyncService::IsSyncActive() const {
  return false;
}

void FakeSyncService::TriggerRefresh(const ModelTypeSet& types) {}

ModelTypeSet FakeSyncService::GetActiveDataTypes() const {
  return ModelTypeSet();
}

SyncClient* FakeSyncService::GetSyncClient() const {
  return nullptr;
}

void FakeSyncService::AddObserver(SyncServiceObserver* observer) {}

void FakeSyncService::RemoveObserver(SyncServiceObserver* observer) {}

bool FakeSyncService::HasObserver(const SyncServiceObserver* observer) const {
  return false;
}

#if defined(OPERA_SYNC)
void FakeSyncService::AddSyncStateObserver(opera::SyncObserver* observer) {
}

void FakeSyncService::RemoveSyncStateObserver(opera::SyncObserver* observer) {
}
#endif  // OPERA_SYNC


bool FakeSyncService::CanSyncStart() const {
  return false;
}

void FakeSyncService::OnDataTypeRequestsSyncStartup(ModelType type) {}

void FakeSyncService::RequestStop(SyncService::SyncStopDataFate data_fate) {}

void FakeSyncService::RequestStart() {}

ModelTypeSet FakeSyncService::GetPreferredDataTypes() const {
  return ModelTypeSet();
}

void FakeSyncService::OnUserChoseDatatypes(bool sync_everything,
                                           ModelTypeSet chosen_types) {}

void FakeSyncService::SetFirstSetupComplete() {}

bool FakeSyncService::IsFirstSetupInProgress() const {
  return false;
}

std::unique_ptr<SyncSetupInProgressHandle>
FakeSyncService::GetSetupInProgressHandle() {
  return nullptr;
}

bool FakeSyncService::IsSetupInProgress() const {
  return false;
}

bool FakeSyncService::ConfigurationDone() const {
  return false;
}

const GoogleServiceAuthError& FakeSyncService::GetAuthError() const {
  return error_;
}

bool FakeSyncService::HasUnrecoverableError() const {
  return false;
}

bool FakeSyncService::IsBackendInitialized() const {
  return false;
}

sync_sessions::OpenTabsUIDelegate* FakeSyncService::GetOpenTabsUIDelegate() {
  return nullptr;
}

bool FakeSyncService::IsPassphraseRequiredForDecryption() const {
  return false;
}

base::Time FakeSyncService::GetExplicitPassphraseTime() const {
  return base::Time();
}

bool FakeSyncService::IsUsingSecondaryPassphrase() const {
  return false;
}

void FakeSyncService::EnableEncryptEverything() {}

bool FakeSyncService::IsEncryptEverythingEnabled() const {
  return false;
}

void FakeSyncService::SetEncryptionPassphrase(const std::string& passphrase,
                                              PassphraseType type) {}

bool FakeSyncService::SetDecryptionPassphrase(const std::string& passphrase) {
  return false;
}

bool FakeSyncService::IsCryptographerReady(const BaseTransaction* trans) const {
  return false;
}

UserShare* FakeSyncService::GetUserShare() const {
  return user_share_.get();
}

LocalDeviceInfoProvider* FakeSyncService::GetLocalDeviceInfoProvider() const {
  return nullptr;
}

void FakeSyncService::RegisterDataTypeController(
    std::unique_ptr<DataTypeController> data_type_controller) {}

void FakeSyncService::ReenableDatatype(ModelType type) {}

bool FakeSyncService::IsPassphraseRequired() const {
  return false;
}

ModelTypeSet FakeSyncService::GetEncryptedDataTypes() const {
  return ModelTypeSet();
}

FakeSyncService::SyncTokenStatus FakeSyncService::GetSyncTokenStatus() const {
  return FakeSyncService::SyncTokenStatus();
}

std::string FakeSyncService::QuerySyncStatusSummaryString() {
  return "";
}

bool FakeSyncService::QueryDetailedSyncStatus(SyncStatus* result) {
  return false;
}

base::string16 FakeSyncService::GetLastSyncedTimeString() const {
  return base::string16();
}

std::string FakeSyncService::GetBackendInitializationStateString() const {
  return std::string();
}

SyncCycleSnapshot FakeSyncService::GetLastCycleSnapshot() const {
  return SyncCycleSnapshot();
}

base::Value* FakeSyncService::GetTypeStatusMap() const {
  return new base::ListValue();
}

const GURL& FakeSyncService::sync_service_url() const {
  return sync_service_url_;
}

std::string FakeSyncService::unrecoverable_error_message() const {
  return unrecoverable_error_message_;
}

tracked_objects::Location FakeSyncService::unrecoverable_error_location()
    const {
  return tracked_objects::Location();
}

void FakeSyncService::AddProtocolEventObserver(
    ProtocolEventObserver* observer) {}

void FakeSyncService::RemoveProtocolEventObserver(
    ProtocolEventObserver* observer) {}

void FakeSyncService::AddTypeDebugInfoObserver(
    TypeDebugInfoObserver* observer) {}

void FakeSyncService::RemoveTypeDebugInfoObserver(
    TypeDebugInfoObserver* observer) {}

base::WeakPtr<JsController> FakeSyncService::GetJsController() {
  return base::WeakPtr<JsController>();
}

void FakeSyncService::GetAllNodes(
    const base::Callback<void(std::unique_ptr<base::ListValue>)>& callback) {}

#if defined(OPERA_SYNC)
// Returns the auth.opera.com account for the sync session.
opera::SyncAccount* FakeSyncService::account() const {
  return nullptr;
}

opera::SyncStatus FakeSyncService::opera_sync_status() {
  opera::SyncServerInfo server_info;
  return opera::SyncStatus(
      0,
      server_info,
      false,
      true,
      false,
      "",
      false,
      false,
      false,
      syncer::ModelTypeSet());
}

void FakeSyncService::OnSyncAuthKeeperInitialized() {
  // Empty.
}

#endif  // OPERA_SYNC

}  // namespace syncer
