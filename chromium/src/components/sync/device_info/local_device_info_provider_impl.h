// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DEVICE_INFO_LOCAL_DEVICE_INFO_PROVIDER_IMPL_H_
#define COMPONENTS_SYNC_DEVICE_INFO_LOCAL_DEVICE_INFO_PROVIDER_IMPL_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "components/sync/device_info/device_info.h"
#include "components/sync/device_info/local_device_info_provider.h"
#include "components/version_info/version_info.h"

namespace syncer {

class LocalDeviceInfoProviderImpl : public LocalDeviceInfoProvider,
                                    public base::NonThreadSafe {
 public:
  LocalDeviceInfoProviderImpl(version_info::Channel channel,
                              const std::string& version,
                              bool is_tablet);
  ~LocalDeviceInfoProviderImpl() override;

  // LocalDeviceInfoProvider implementation.
  const DeviceInfo* GetLocalDeviceInfo() const override;
  std::string GetSyncUserAgent() const override;
  std::string GetLocalSyncCacheGUID() const override;
  void Initialize(
      const std::string& cache_guid,
      const std::string& signin_scoped_device_id,
      const scoped_refptr<base::TaskRunner>& blocking_task_runner) override;
  std::unique_ptr<Subscription> RegisterOnInitializedCallback(
      const base::Closure& callback) override;
  void Clear() override;

 private:
  void InitializeContinuation(const std::string& guid,
                              const std::string& signin_scoped_device_id,
                              const std::string& session_name);

#if !defined(OPERA_SYNC)
  // The channel (CANARY, DEV, BETA, etc.) of the current client.
  const version_info::Channel channel_;
#endif  // !OPERA_SYNC

  // The version string for the current client.
  const std::string version_;

  // Whether this device has a tablet form factor (only used on Android
  // devices).
  const bool is_tablet_;

  std::string cache_guid_;
  std::unique_ptr<DeviceInfo> local_device_info_;
  base::CallbackList<void(void)> callback_list_;
  base::WeakPtrFactory<LocalDeviceInfoProviderImpl> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(LocalDeviceInfoProviderImpl);
};

}  // namespace syncer

#endif  // COMPONENTS_SYNC_DEVICE_INFO_LOCAL_DEVICE_INFO_PROVIDER_IMPL_H_
