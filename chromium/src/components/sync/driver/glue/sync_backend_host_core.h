// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_GLUE_SYNC_BACKEND_HOST_CORE_H_
#define COMPONENTS_SYNC_DRIVER_GLUE_SYNC_BACKEND_HOST_CORE_H_

#if defined(OPERA_SYNC)
#include <map>
#endif  // OPERA_SYNC
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/timer/timer.h"
#include "components/invalidation/public/invalidation.h"
#include "components/sync/base/cancelation_signal.h"
#include "components/sync/core/shutdown_reason.h"
#include "components/sync/core/sync_encryption_handler.h"
#include "components/sync/driver/glue/sync_backend_host_impl.h"
#include "components/sync/driver/system_encryptor.h"
#include "components/sync/engine/cycle/type_debug_info_observer.h"
#include "url/gurl.h"

namespace syncer {

class SyncBackendHostImpl;

// Utility struct for holding initialization options.
struct DoInitializeOptions {
  DoInitializeOptions(
      base::MessageLoop* sync_loop,
      SyncBackendRegistrar* registrar,
      const std::vector<scoped_refptr<ModelSafeWorker>>& workers,
      const scoped_refptr<ExtensionsActivity>& extensions_activity,
      const WeakHandle<JsEventHandler>& event_handler,
      const GURL& service_url,
      const std::string& sync_user_agent,
      std::unique_ptr<HttpPostProviderFactory> http_bridge_factory,
      const SyncCredentials& credentials,
      const std::string& invalidator_client_id,
      std::unique_ptr<SyncManagerFactory> sync_manager_factory,
      bool delete_sync_data_folder,
      const std::string& restored_key_for_bootstrapping,
      const std::string& restored_keystore_key_for_bootstrapping,
      std::unique_ptr<InternalComponentsFactory> internal_components_factory,
      const WeakHandle<UnrecoverableErrorHandler>& unrecoverable_error_handler,
      const base::Closure& report_unrecoverable_error_function,
#if defined(OPERA_SYNC)
      const sync_oauth_signer::SignParams& sign_params,
#endif  // OPERA_SYNC
      std::unique_ptr<SyncEncryptionHandler::NigoriState> saved_nigori_state,
      const std::map<ModelType, int64_t>& invalidation_versions);
  ~DoInitializeOptions();

  base::MessageLoop* sync_loop;
  SyncBackendRegistrar* registrar;
  std::vector<scoped_refptr<ModelSafeWorker>> workers;
  scoped_refptr<ExtensionsActivity> extensions_activity;
  WeakHandle<JsEventHandler> event_handler;
  GURL service_url;
  std::string sync_user_agent;
  // Overridden by tests.
  std::unique_ptr<HttpPostProviderFactory> http_bridge_factory;
  SyncCredentials credentials;
  const std::string invalidator_client_id;
  std::unique_ptr<SyncManagerFactory> sync_manager_factory;
  std::string lsid;
  bool delete_sync_data_folder;
  std::string restored_key_for_bootstrapping;
  std::string restored_keystore_key_for_bootstrapping;
  std::unique_ptr<InternalComponentsFactory> internal_components_factory;
  const WeakHandle<UnrecoverableErrorHandler> unrecoverable_error_handler;
#if defined(OPERA_SYNC)
  sync_oauth_signer::SignParams sign_params;
#endif  // OPERA_SYNC
  base::Closure report_unrecoverable_error_function;
  std::unique_ptr<SyncEncryptionHandler::NigoriState> saved_nigori_state;
  const std::map<ModelType, int64_t> invalidation_versions;
};

// Helper struct to handle currying params to
// SyncBackendHost::Core::DoConfigureSyncer.
struct DoConfigureSyncerTypes {
  DoConfigureSyncerTypes();
  DoConfigureSyncerTypes(const DoConfigureSyncerTypes& other);
  ~DoConfigureSyncerTypes();
  ModelTypeSet to_download;
  ModelTypeSet to_purge;
  ModelTypeSet to_journal;
  ModelTypeSet to_unapply;
};

class SyncBackendHostCore
    : public base::RefCountedThreadSafe<SyncBackendHostCore>,
      public SyncEncryptionHandler::Observer,
      public SyncManager::Observer,
      public TypeDebugInfoObserver {
 public:
  SyncBackendHostCore(const std::string& name,
                      const base::FilePath& sync_data_folder_path,
                      bool has_sync_setup_completed,
                      const base::WeakPtr<SyncBackendHostImpl>& backend);

  // SyncManager::Observer implementation.  The Core just acts like an air
  // traffic controller here, forwarding incoming messages to appropriate
  // landing threads.
  void OnSyncCycleCompleted(const SyncCycleSnapshot& snapshot) override;
  void OnInitializationComplete(
      const WeakHandle<JsBackend>& js_backend,
      const WeakHandle<DataTypeDebugInfoListener>& debug_info_listener,
      bool success,
      ModelTypeSet restored_types) override;
  void OnConnectionStatusChange(ConnectionStatus status) override;
  void OnActionableError(const SyncProtocolError& sync_error) override;
  void OnMigrationRequested(ModelTypeSet types) override;
  void OnProtocolEvent(const ProtocolEvent& event) override;

  // SyncEncryptionHandler::Observer implementation.
  void OnPassphraseRequired(
      PassphraseRequiredReason reason,
      const sync_pb::EncryptedData& pending_keys) override;
  void OnPassphraseAccepted() override;
  void OnBootstrapTokenUpdated(const std::string& bootstrap_token,
                               BootstrapTokenType type) override;
  void OnEncryptedTypesChanged(ModelTypeSet encrypted_types,
                               bool encrypt_everything) override;
  void OnEncryptionComplete() override;
  void OnCryptographerStateChanged(Cryptographer* cryptographer) override;
  void OnPassphraseTypeChanged(PassphraseType type,
                               base::Time passphrase_time) override;
  void OnLocalSetPassphraseEncryption(
      const SyncEncryptionHandler::NigoriState& nigori_state) override;

  // TypeDebugInfoObserver implementation
  void OnCommitCountersUpdated(ModelType type,
                               const CommitCounters& counters) override;
  void OnUpdateCountersUpdated(ModelType type,
                               const UpdateCounters& counters) override;
  void OnStatusCountersUpdated(ModelType type,
                               const StatusCounters& counters) override;

  // Forwards an invalidation state change to the sync manager.
#if defined(OPERA_SYNC)
  void DoOnInvalidatorStateChange(OperaInvalidatorState state);
#else
  void DoOnInvalidatorStateChange(InvalidatorState state);
#endif  // OPERA_SYNC

  // Forwards an invalidation to the sync manager.
  void DoOnIncomingInvalidation(
      const ObjectIdInvalidationMap& invalidation_map);

  // Note:
  //
  // The Do* methods are the various entry points from our
  // SyncBackendHost.  They are all called on the sync thread to
  // actually perform synchronous (and potentially blocking) syncapi
  // operations.
  //
  // Called to perform initialization of the syncapi on behalf of
  // SyncBackendHost::Initialize.
  void DoInitialize(std::unique_ptr<DoInitializeOptions> options);

  // Called to perform credential update on behalf of
  // SyncBackendHost::UpdateCredentials.
  void DoUpdateCredentials(const SyncCredentials& credentials);

  // Called to tell the syncapi to start syncing (generally after
  // initialization and authentication).
  void DoStartSyncing(const ModelSafeRoutingInfo& routing_info,
                      base::Time last_poll_time);

  // Called to set the passphrase for encryption.
  void DoSetEncryptionPassphrase(const std::string& passphrase,
                                 bool is_explicit);

  // Called to decrypt the pending keys.
  void DoSetDecryptionPassphrase(const std::string& passphrase);

  // Called to turn on encryption of all sync data as well as
  // reencrypt everything.
  void DoEnableEncryptEverything();

  // Ask the syncer to check for updates for the specified types.
  void DoRefreshTypes(ModelTypeSet types);

  // Invoked if we failed to download the necessary control types at startup.
  // Invokes SyncBackendHost::HandleControlTypesDownloadRetry.
  void OnControlTypesDownloadRetry();

  // Called to perform tasks which require the control data to be downloaded.
  // This includes refreshing encryption, etc.
  void DoInitialProcessControlTypes();

  // The shutdown order is a bit complicated:
  // 1) Call ShutdownOnUIThread() from |frontend_loop_| to request sync manager
  //    to stop as soon as possible.
  // 2) Post DoShutdown() to sync loop to clean up backend state, save
  //    directory and destroy sync manager.
  void ShutdownOnUIThread();
  void DoShutdown(ShutdownReason reason);
  void DoDestroySyncManager(ShutdownReason reason);

  // Configuration methods that must execute on sync loop.
  void DoConfigureSyncer(
      ConfigureReason reason,
      const DoConfigureSyncerTypes& config_types,
      const ModelSafeRoutingInfo routing_info,
      const base::Callback<void(ModelTypeSet, ModelTypeSet)>& ready_task,
      const base::Closure& retry_callback);
  void DoFinishConfigureDataTypes(
      ModelTypeSet types_to_config,
      const base::Callback<void(ModelTypeSet, ModelTypeSet)>& ready_task);
  void DoRetryConfiguration(const base::Closure& retry_callback);

  // Set the base request context to use when making HTTP calls.
  // This method will add a reference to the context to persist it
  // on the IO thread. Must be removed from IO thread.

  SyncManager* sync_manager() { return sync_manager_.get(); }

  void SendBufferedProtocolEventsAndEnableForwarding();
  void DisableProtocolEventForwarding();

  // Enables the forwarding of directory type debug counters to the
  // SyncBackendHost.  Also requests that updates to all counters be
  // emitted right away to initialize any new listeners' states.
  void EnableDirectoryTypeDebugInfoForwarding();

  // Disables forwarding of directory type debug counters.
  void DisableDirectoryTypeDebugInfoForwarding();

  // Delete the sync data folder to cleanup backend data.  Happens the first
  // time sync is enabled for a user (to prevent accidentally reusing old
  // sync databases), as well as shutdown when you're no longer syncing.
  void DeleteSyncDataFolder();

  // We expose this member because it's required in the construction of the
  // HttpBridgeFactory.
  CancelationSignal* GetRequestContextCancelationSignal() {
    return &release_request_context_signal_;
  }

#if defined(OPERA_SYNC)
  // Opera-specific. Compare ProfileSyncService::invalidation_map().
  syncer::VersionMap invalidation_map() const;
#endif  // OPERA_SYNC

  // Tell the sync manager to persist its state by writing to disk.
  // Called on the sync thread, both by a timer and, on Android, when the
  // application is backgrounded.
  void SaveChanges();

  void DoClearServerData(
      const SyncManager::ClearServerDataCallback& frontend_callback);

  // Notify the syncer that the cookie jar has changed.
  void DoOnCookieJarChanged(bool account_mismatch, bool empty_jar);

 private:
  friend class base::RefCountedThreadSafe<SyncBackendHostCore>;

  ~SyncBackendHostCore() override;

  // Invoked when initialization of syncapi is complete and we can start
  // our timer.
  // This must be called from the thread on which SaveChanges is intended to
  // be run on; the host's |registrar_->sync_thread()|.
  void StartSavingChanges();

  void ClearServerDataDone(const base::Closure& frontend_callback);

  // Name used for debugging.
  const std::string name_;

  // Path of the folder that stores the sync data files.
  const base::FilePath sync_data_folder_path_;

  // Our parent SyncBackendHost.
  WeakHandle<SyncBackendHostImpl> host_;

  // The loop where all the sync backend operations happen.
  // Non-NULL only between calls to DoInitialize() and ~Core().
  base::MessageLoop* sync_loop_;

  // Our parent's registrar (not owned).  Non-NULL only between
  // calls to DoInitialize() and DoShutdown().
  SyncBackendRegistrar* registrar_;

  // The timer used to periodically call SaveChanges.
  std::unique_ptr<base::RepeatingTimer> save_changes_timer_;

  // Our encryptor, which uses Chrome's encryption functions.
  SystemEncryptor encryptor_;

  // The top-level syncapi entry point.  Lives on the sync thread.
  std::unique_ptr<SyncManager> sync_manager_;

  // Temporary holder of sync manager's initialization results. Set by
  // OnInitializeComplete, and consumed when we pass it via OnBackendInitialized
  // in the final state of HandleInitializationSuccessOnFrontendLoop.
  WeakHandle<JsBackend> js_backend_;
  WeakHandle<DataTypeDebugInfoListener> debug_info_listener_;

  // These signals allow us to send requests to shut down the HttpBridgeFactory
  // and ServerConnectionManager without having to wait for those classes to
  // finish initializing first.
  //
  // See comments in SyncBackendHostCore::ShutdownOnUIThread() for more details.
  CancelationSignal release_request_context_signal_;
  CancelationSignal stop_syncing_signal_;

#if defined(OPERA_SYNC)
  // Opera-specific. Compare ProfileSyncService::invalidation_map().
  syncer::VersionMap invalidation_map_;
#endif  // OPERA_SYNC

  // Matches the value of SyncPref's IsFirstSetupComplete() flag at init time.
  // Should not be used for anything except for UMAs and logging.
  const bool has_sync_setup_completed_;

  // Set when we've been asked to forward sync protocol events to the frontend.
  bool forward_protocol_events_;

  // Set when the forwarding of per-type debug counters is enabled.
  bool forward_type_info_;

  // A map of data type -> invalidation version to track the most recently
  // received invalidation version for each type.
  // This allows dropping any invalidations with versions older than those
  // most recently received for that data type.
  std::map<ModelType, int64_t> last_invalidation_versions_;

  base::WeakPtrFactory<SyncBackendHostCore> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(SyncBackendHostCore);
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_DRIVER_GLUE_SYNC_BACKEND_HOST_CORE_H_
