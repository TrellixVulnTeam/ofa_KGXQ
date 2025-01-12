// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/driver/fake_sync_client.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "components/sync/base/extensions_activity.h"
#include "components/sync/driver/fake_sync_service.h"
#include "components/sync/driver/sync_prefs.h"

namespace syncer {

namespace {

void DummyRegisterPlatformTypesCallback(SyncService* sync_service,
                                        ModelTypeSet,
                                        ModelTypeSet) {}

}  // namespace

FakeSyncClient::FakeSyncClient()
    : model_type_service_(nullptr),
      factory_(nullptr),
      sync_service_(base::MakeUnique<FakeSyncService>()) {
  // Register sync preferences and set them to "Sync everything" state.
  SyncPrefs::RegisterProfilePrefs(pref_service_.registry());
  SyncPrefs sync_prefs(GetPrefService());
  sync_prefs.SetFirstSetupComplete();
  sync_prefs.SetKeepEverythingSynced(true);
}

FakeSyncClient::FakeSyncClient(SyncApiComponentFactory* factory)
    : factory_(factory), sync_service_(base::MakeUnique<FakeSyncService>()) {
  SyncPrefs::RegisterProfilePrefs(pref_service_.registry());
}

FakeSyncClient::~FakeSyncClient() {}

void FakeSyncClient::Initialize() {}

SyncService* FakeSyncClient::GetSyncService() {
  return sync_service_.get();
}

PrefService* FakeSyncClient::GetPrefService() {
  return &pref_service_;
}

bookmarks::BookmarkModel* FakeSyncClient::GetBookmarkModel() {
  return nullptr;
}

favicon::FaviconService* FakeSyncClient::GetFaviconService() {
  return nullptr;
}

history::HistoryService* FakeSyncClient::GetHistoryService() {
  return nullptr;
}

#if defined(OPERA_SYNC)
opera::DuplicateTrackerInterface*
FakeSyncClient::GetBookmarkDuplicateTracker() {
  return nullptr;
}

opera::SyncAccount* FakeSyncClient::CreateSyncAccount(
    std::unique_ptr<opera::SyncLoginDataStore> login_data_store,
    opera::SyncAccount::AuthDataUpdaterFactory auth_data_updater_factory,
    opera::TimeSkewResolverFactory time_skew_resolver_factory,
    const base::Closure& stop_syncing_callback) {
  return nullptr;
}

opera::SyncAccount* FakeSyncClient::GetSyncAccount() {
  return nullptr;
}

opera::SyncErrorHandler* FakeSyncClient::CreateSyncErrorHandler(
    base::TimeDelta show_status_delay,
    std::unique_ptr<opera::SyncDelayProvider> delay_provider,
    const opera::ShowSyncConnectionStatusCallback& show_status_callback,
    const opera::ShowSyncLoginPromptCallback& show_login_prompt_callback) {
  return nullptr;
}

#if defined(OPERA_DESKTOP)
opera::sync::SyncAuthKeeper* FakeSyncClient::GetOperaSyncAuthKeeper() {
  return nullptr;
}

void FakeSyncClient::OnSyncAuthKeeperInitialized(
  opera::sync::SyncAuthKeeper* keeper) {
  // Empty.
}

LoginUIService::LoginUI* FakeSyncClient::GetLoginUI() {
  return nullptr;
}

opera::OtherSpeedDialsCleaner* FakeSyncClient::GetOtherSpeeddialsCleaner() {
  return nullptr;
}

scoped_refptr<opera::SyncPasswordRecoverer>
    FakeSyncClient::CreateOperaSyncPasswordRecoverer() {
  return nullptr;
}

opera::oauth2::AuthService* FakeSyncClient::GetOperaOAuth2Service() {
  return nullptr;
}
#endif  // OPERA_DESKTOP
#endif  // OPERA_SYNC

base::Closure FakeSyncClient::GetPasswordStateChangedCallback() {
  return base::Bind(&base::DoNothing);
}

SyncApiComponentFactory::RegisterDataTypesMethod
FakeSyncClient::GetRegisterPlatformTypesCallback() {
  return base::Bind(&DummyRegisterPlatformTypesCallback);
}

autofill::PersonalDataManager* FakeSyncClient::GetPersonalDataManager() {
  return nullptr;
}

BookmarkUndoService* FakeSyncClient::GetBookmarkUndoServiceIfExists() {
  return nullptr;
}

invalidation::InvalidationService* FakeSyncClient::GetInvalidationService() {
  return nullptr;
}

scoped_refptr<ExtensionsActivity> FakeSyncClient::GetExtensionsActivity() {
  return scoped_refptr<ExtensionsActivity>();
}

sync_sessions::SyncSessionsClient* FakeSyncClient::GetSyncSessionsClient() {
  return nullptr;
}

base::WeakPtr<SyncableService> FakeSyncClient::GetSyncableServiceForType(
    ModelType type) {
  return base::WeakPtr<SyncableService>();
}

base::WeakPtr<ModelTypeService> FakeSyncClient::GetModelTypeServiceForType(
    ModelType type) {
  return model_type_service_->AsWeakPtr();
}

scoped_refptr<ModelSafeWorker> FakeSyncClient::CreateModelWorkerForGroup(
    ModelSafeGroup group,
    WorkerLoopDestructionObserver* observer) {
  return scoped_refptr<ModelSafeWorker>();
}

SyncApiComponentFactory* FakeSyncClient::GetSyncApiComponentFactory() {
  return factory_;
}

void FakeSyncClient::SetModelTypeService(ModelTypeService* model_type_service) {
  model_type_service_ = model_type_service;
}

}  // namespace syncer
