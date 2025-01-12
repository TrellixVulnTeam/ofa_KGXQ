// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/opera/android/browser/WebContentsWrapper

#ifndef com_opera_android_browser_WebContentsWrapper_JNI
#define com_opera_android_browser_WebContentsWrapper_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kWebContentsWrapperClassPath[] =
    "com/opera/android/browser/WebContentsWrapper";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_WebContentsWrapper_clazz __attribute__((unused)) = 0;
#define WebContentsWrapper_clazz(env) base::android::LazyGetClass(env, kWebContentsWrapperClassPath, &g_WebContentsWrapper_clazz)

}  // namespace

namespace opera {

// Step 2: method stubs.

static jlong CreateWebContentsWrapper(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& webContents);

JNI_GENERATOR_EXPORT jlong
    Java_com_opera_android_browser_WebContentsWrapper_nativeCreateWebContentsWrapper(JNIEnv*
    env, jobject jcaller,
    jobject webContents) {
  return CreateWebContentsWrapper(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, webContents));
}

JNI_GENERATOR_EXPORT void
    Java_com_opera_android_browser_WebContentsWrapper_nativeDestroy(JNIEnv* env,
    jobject jcaller,
    jlong nativeWebContentsWrapper) {
  WebContentsWrapper* native =
      reinterpret_cast<WebContentsWrapper*>(nativeWebContentsWrapper);
  CHECK_NATIVE_PTR(env, jcaller, native, "Destroy");
  return native->Destroy(env, base::android::JavaParamRef<jobject>(env,
      jcaller));
}

JNI_GENERATOR_EXPORT jobject
    Java_com_opera_android_browser_WebContentsWrapper_nativeGetJavaWebContents(JNIEnv*
    env, jobject jcaller,
    jlong nativeWebContentsWrapper) {
  WebContentsWrapper* native =
      reinterpret_cast<WebContentsWrapper*>(nativeWebContentsWrapper);
  CHECK_NATIVE_PTR(env, jcaller, native, "GetJavaWebContents", NULL);
  return native->GetJavaWebContents(env,
      base::android::JavaParamRef<jobject>(env, jcaller)).Release();
}

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsWebContentsWrapper[] = {
    { "nativeCreateWebContentsWrapper",
"("
"Lorg/chromium/content_public/browser/WebContents;"
")"
"J",
    reinterpret_cast<void*>(Java_com_opera_android_browser_WebContentsWrapper_nativeCreateWebContentsWrapper)
    },
    { "nativeDestroy",
"("
"J"
")"
"V",
    reinterpret_cast<void*>(Java_com_opera_android_browser_WebContentsWrapper_nativeDestroy)
    },
    { "nativeGetJavaWebContents",
"("
"J"
")"
"Lorg/chromium/content_public/browser/WebContents;",
    reinterpret_cast<void*>(Java_com_opera_android_browser_WebContentsWrapper_nativeGetJavaWebContents)
    },
};

static bool RegisterNativesImpl(JNIEnv* env) {
  if (base::android::IsManualJniRegistrationDisabled()) return true;

  const int kMethodsWebContentsWrapperSize =
      arraysize(kMethodsWebContentsWrapper);

  if (env->RegisterNatives(WebContentsWrapper_clazz(env),
                           kMethodsWebContentsWrapper,
                           kMethodsWebContentsWrapperSize) < 0) {
    jni_generator::HandleRegistrationError(
        env, WebContentsWrapper_clazz(env), __FILE__);
    return false;
  }

  return true;
}

}  // namespace opera

#endif  // com_opera_android_browser_WebContentsWrapper_JNI
