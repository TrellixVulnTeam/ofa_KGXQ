// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_URL_UTILS_H_
#define CONTENT_PUBLIC_COMMON_URL_UTILS_H_

#include "content/common/content_export.h"

class GURL;

namespace content {

// Null terminated list of schemes that are savable. This function can be
// invoked on any thread.
CONTENT_EXPORT const char* const* GetSavableSchemes();

// Returns true if the url has a scheme for WebUI.  See also
// WebUIControllerFactory::UseWebUIForURL in the browser process.
CONTENT_EXPORT bool HasWebUIScheme(const GURL& url);

// Check whether we can do the saving page operation for the specified URL.
CONTENT_EXPORT bool IsSavableURL(const GURL& url);

#if defined(OPERA_DESKTOP) || defined(OPERA_TVSDK) || defined(OPERA_WAM)
CONTENT_EXPORT void ReplaceLegacyOperaScheme(GURL* url);
#endif  // defined(OPERA_DESKTOP) || defined(OPERA_TVSDK) || defined(OPERA_WAM)

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_URL_UTILS_H_
