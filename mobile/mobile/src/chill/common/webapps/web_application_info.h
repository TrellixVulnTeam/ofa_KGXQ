// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/common/web_application_info.h
// @final-synchronized

#ifndef CHILL_COMMON_WEBAPPS_WEB_APPLICATION_INFO_H_
#define CHILL_COMMON_WEBAPPS_WEB_APPLICATION_INFO_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/geometry/size.h"
#include "url/gurl.h"

namespace opera {

const int kMaxMetaTagAttributeLength = 2000;

// Structure used when installing a web page as an app.
struct WebApplicationInfo {
  struct IconInfo {
    IconInfo();
    ~IconInfo();

    GURL url;
    int width;
    int height;
    SkBitmap data;
  };

  enum MobileCapable {
    MOBILE_CAPABLE_UNSPECIFIED,
    MOBILE_CAPABLE,
    MOBILE_CAPABLE_APPLE
  };

  WebApplicationInfo();
  ~WebApplicationInfo();

  // Title of the application.
  base::string16 title;

  // Description of the application.
  base::string16 description;

  // The launch URL for the app.
  GURL app_url;

  // Set of available icons.
  std::vector<IconInfo> icons;

  // Whether the page is marked as mobile-capable, including apple specific meta
  // tag.
  MobileCapable mobile_capable;

  // The color to use if an icon needs to be generated for the web app.
  SkColor generated_icon_color;
};

}  // namespace opera

#endif  // CHILL_COMMON_WEBAPPS_WEB_APPLICATION_INFO_H_
