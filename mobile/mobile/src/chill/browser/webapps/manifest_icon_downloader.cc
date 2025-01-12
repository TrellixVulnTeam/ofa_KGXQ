// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modified by Opera Software ASA
// @copied-from chrome/browser/manifest/manifest_icon_downloader.cc
// @final-synchronized

#include "chill/browser/webapps/manifest_icon_downloader.h"

#include <limits>

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "skia/ext/image_operations.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"

bool ManifestIconDownloader::Download(
    content::WebContents* web_contents,
    const GURL& icon_url,
    int ideal_icon_size_in_dp,
    int minimum_icon_size_in_dp,
    const ManifestIconDownloader::IconFetchCallback& callback) {
  DCHECK(minimum_icon_size_in_dp <= ideal_icon_size_in_dp);
  if (!web_contents || !icon_url.is_valid())
    return false;

  const float device_scale_factor =
      display::Screen::GetScreen()->GetPrimaryDisplay().device_scale_factor();
  const int ideal_icon_size_in_px =
      static_cast<int>(round(ideal_icon_size_in_dp * device_scale_factor));
  const int minimum_icon_size_in_px =
      static_cast<int>(round(minimum_icon_size_in_dp * device_scale_factor));

  web_contents->DownloadImage(
      icon_url,
      false,  // is_favicon
      0,      // max_bitmap_size - 0 means no maximum size.
      false,  // bypass_cache
      base::Bind(&ManifestIconDownloader::OnIconFetched,
                 ideal_icon_size_in_px,
                 minimum_icon_size_in_px,
                 callback));
  return true;
}

void ManifestIconDownloader::OnIconFetched(
    int ideal_icon_size_in_px,
    int minimum_icon_size_in_px,
    const ManifestIconDownloader::IconFetchCallback& callback,
    int id,
    int http_status_code,
    const GURL& url,
    const std::vector<SkBitmap>& bitmaps,
    const std::vector<gfx::Size>& sizes) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  const int closest_index = FindClosestBitmapIndex(
      ideal_icon_size_in_px, minimum_icon_size_in_px, bitmaps);

  if (closest_index == -1) {
    callback.Run(SkBitmap());
    return;
  }

  const SkBitmap& chosen = bitmaps[closest_index];

  // Only scale if we need to scale down. For scaling up we will let the system
  // handle that when it is required to display it. This saves space in the
  // webapp storage system as well.
  if (chosen.height() > ideal_icon_size_in_px) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&ManifestIconDownloader::ScaleIcon,
                   ideal_icon_size_in_px,
                   chosen,
                   callback));
    return;
  }

  callback.Run(chosen);
}

void ManifestIconDownloader::ScaleIcon(
    int ideal_icon_size_in_px,
    const SkBitmap& bitmap,
    const ManifestIconDownloader::IconFetchCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  const SkBitmap& scaled = skia::ImageOperations::Resize(
      bitmap,
      skia::ImageOperations::RESIZE_BEST,
      ideal_icon_size_in_px,
      ideal_icon_size_in_px);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(callback, scaled));
}

int ManifestIconDownloader::FindClosestBitmapIndex(
    int ideal_icon_size_in_px,
    int minimum_icon_size_in_px,
    const std::vector<SkBitmap>& bitmaps) {
  int best_index = -1;
  int best_delta = std::numeric_limits<int>::min();
  const int max_negative_delta =
      minimum_icon_size_in_px - ideal_icon_size_in_px;

  for (size_t i = 0; i < bitmaps.size(); ++i) {
    if (bitmaps[i].height() != bitmaps[i].width())
      continue;

    int delta = bitmaps[i].width() - ideal_icon_size_in_px;
    if (delta == 0)
      return i;

    if (best_delta > 0 && delta < 0)
      continue;

    if ((best_delta > 0 && delta < best_delta) ||
        (best_delta < 0 && delta > best_delta && delta >= max_negative_delta)) {
      best_index = i;
      best_delta = delta;
    }
  }
  return best_index;
}
