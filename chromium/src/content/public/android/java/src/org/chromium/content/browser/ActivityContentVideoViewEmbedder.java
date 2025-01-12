// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Build;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

/**
 * Uses an existing Activity to handle displaying video in full screen.
 */
public class ActivityContentVideoViewEmbedder implements ContentVideoViewEmbedder {
    private final Activity mActivity;
    private View mView;

    // The activity orientation before entering fullscreen.
    private int mPreviousOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;

    public ActivityContentVideoViewEmbedder(Activity activity)  {
        this.mActivity = activity;
    }

    @Override
    public void enterFullscreenVideo(View view, boolean isVideoLoaded) {
        mPreviousOrientation = mActivity.getRequestedOrientation();
        mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR);

        FrameLayout decor = (FrameLayout) mActivity.getWindow().getDecorView();
        decor.addView(view, 0,
                new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        Gravity.CENTER));
        setSystemUiVisibility(true);
        mView = view;
    }

    @Override
    public void fullscreenVideoLoaded() {}

    @Override
    @SuppressLint("WrongConstant")
    public void exitFullscreenVideo() {
        if (mPreviousOrientation != ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED) {
            mActivity.setRequestedOrientation(mPreviousOrientation);
        }
        FrameLayout decor = (FrameLayout) mActivity.getWindow().getDecorView();
        decor.removeView(mView);
        setSystemUiVisibility(false);
        mView = null;
    }

    @Override
    @SuppressLint("InlinedApi")
    public void setSystemUiVisibility(boolean enterFullscreen) {
        View decor = mActivity.getWindow().getDecorView();
        if (enterFullscreen) {
            mActivity.getWindow().setFlags(
                    WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            mActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return;
        }

        int systemUiVisibility = decor.getSystemUiVisibility();
        int flags = View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        if (enterFullscreen) {
            systemUiVisibility |= flags;
        } else {
            systemUiVisibility &= ~flags;
        }
        decor.setSystemUiVisibility(systemUiVisibility);
    }
}
