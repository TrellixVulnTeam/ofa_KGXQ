// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/browser/content_settings/host_content_settings_map_factory.h
// @final-synchronized

#ifndef CHILL_BROWSER_CONTENT_SETTINGS_HOST_CONTENT_SETTINGS_MAP_FACTORY_H_
#define CHILL_BROWSER_CONTENT_SETTINGS_HOST_CONTENT_SETTINGS_MAP_FACTORY_H_

#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/refcounted_browser_context_keyed_service_factory.h"
#include "base/threading/sequenced_worker_pool.h"

class HostContentSettingsMap;
class PrefRegistry;
class PrefService;

namespace content {
class BrowserContext;
}

class HostContentSettingsMapFactory
    : public RefcountedBrowserContextKeyedServiceFactory {
 public:
  static HostContentSettingsMap* GetForForBrowserContext(
      content::BrowserContext* context);
  static HostContentSettingsMapFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<HostContentSettingsMapFactory>;

  HostContentSettingsMapFactory();
  ~HostContentSettingsMapFactory() override;

  // RefcountedBrowserContextKeyedServiceFactory methods:
  scoped_refptr<RefcountedKeyedService> BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(HostContentSettingsMapFactory);
};

#endif  // CHILL_BROWSER_CONTENT_SETTINGS_HOST_CONTENT_SETTINGS_MAP_FACTORY_H_
