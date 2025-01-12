// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_FAKE_SYNC_CLIENT_H_
#define COMPONENTS_SYNC_DRIVER_FAKE_SYNC_CLIENT_H_

#include <memory>

#include "base/macros.h"
#include "components/sync/driver/sync_client.h"
#include "components/syncable_prefs/testing_pref_service_syncable.h"

#if defined(OPERA_SYNC)
namespace opera {
class DuplicateTracker;
}
#endif  // OPERA_SYNC

namespace syncer {

class FakeSyncService;

// Fake implementation of SyncClient interface for tests.
class FakeSyncClient : public SyncClient {
 public:
  FakeSyncClient();
  explicit FakeSyncClient(SyncApiComponentFactory* factory);
  ~FakeSyncClient() override;

  void Initialize() override;

  SyncService* GetSyncService() override;
  PrefService* GetPrefService() override;
  bookmarks::BookmarkModel* GetBookmarkModel() override;
  favicon::FaviconService* GetFaviconService() override;
  history::HistoryService* GetHistoryService() override;
#if defined(OPERA_SYNC)
  opera::DuplicateTrackerInterface* GetBookmarkDuplicateTracker() override;
  opera::SyncAccount* CreateSyncAccount(
      std::unique_ptr<opera::SyncLoginDataStore> login_data_store,
      opera::SyncAccount::AuthDataUpdaterFactory auth_data_updater_factory,
      opera::TimeSkewResolverFactory time_skew_resolver_factory,
      const base::Closure& stop_syncing_callback) override;
  opera::SyncAccount* GetSyncAccount() override;
  opera::SyncErrorHandler* CreateSyncErrorHandler(
      base::TimeDelta show_status_delay,
      std::unique_ptr<opera::SyncDelayProvider> delay_provider,
      const opera::ShowSyncConnectionStatusCallback& show_status_callback,
      const opera::ShowSyncLoginPromptCallback& show_login_prompt_callback) override;
#if defined(OPERA_DESKTOP)
  opera::oauth2::AuthService* GetOperaOAuth2Service() override;
  opera::sync::SyncAuthKeeper* GetOperaSyncAuthKeeper() override;
  void OnSyncAuthKeeperInitialized(
      opera::sync::SyncAuthKeeper* keeper) override;
  LoginUIService::LoginUI* GetLoginUI() override;
  opera::OtherSpeedDialsCleaner* GetOtherSpeeddialsCleaner() override;
  scoped_refptr<opera::SyncPasswordRecoverer>
      CreateOperaSyncPasswordRecoverer() override;
#endif  // OPERA_DESKTOP
#endif  // OPERA_SYNC
  base::Closure GetPasswordStateChangedCallback() override;
  SyncApiComponentFactory::RegisterDataTypesMethod
  GetRegisterPlatformTypesCallback() override;
  autofill::PersonalDataManager* GetPersonalDataManager() override;
  BookmarkUndoService* GetBookmarkUndoServiceIfExists() override;
  invalidation::InvalidationService* GetInvalidationService() override;
  scoped_refptr<ExtensionsActivity> GetExtensionsActivity() override;
  sync_sessions::SyncSessionsClient* GetSyncSessionsClient() override;
  base::WeakPtr<SyncableService> GetSyncableServiceForType(
      ModelType type) override;
  base::WeakPtr<ModelTypeService> GetModelTypeServiceForType(
      ModelType type) override;
  scoped_refptr<ModelSafeWorker> CreateModelWorkerForGroup(
      ModelSafeGroup group,
      WorkerLoopDestructionObserver* observer) override;
  SyncApiComponentFactory* GetSyncApiComponentFactory() override;

  void SetModelTypeService(ModelTypeService* model_type_service);

 private:
  syncable_prefs::TestingPrefServiceSyncable pref_service_;
  ModelTypeService* model_type_service_;
  SyncApiComponentFactory* factory_;
  std::unique_ptr<FakeSyncService> sync_service_;

  DISALLOW_COPY_AND_ASSIGN(FakeSyncClient);
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_DRIVER_FAKE_SYNC_CLIENT_H_
