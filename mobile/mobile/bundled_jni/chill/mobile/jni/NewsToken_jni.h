// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/opera/android/dashboard/newsfeed/auth/NewsToken

#ifndef com_opera_android_dashboard_newsfeed_auth_NewsToken_JNI
#define com_opera_android_dashboard_newsfeed_auth_NewsToken_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kNewsTokenClassPath[] =
    "com/opera/android/dashboard/newsfeed/auth/NewsToken";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_NewsToken_clazz __attribute__((unused)) = 0;
#define NewsToken_clazz(env) base::android::LazyGetClass(env, kNewsTokenClassPath, &g_NewsToken_clazz)

}  // namespace

namespace opera {

// Step 2: method stubs.

static base::subtle::AtomicWord g_NewsToken_create = 0;
static base::android::ScopedJavaLocalRef<jobject> Java_NewsToken_create(JNIEnv*
    env, const base::android::JavaRefOrBare<jstring>& token,
    jlong expires) {
  CHECK_CLAZZ(env, NewsToken_clazz(env),
      NewsToken_clazz(env), NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, NewsToken_clazz(env),
      "create",

"("
"Ljava/lang/String;"
"J"
")"
"Lcom/opera/android/dashboard/newsfeed/auth/NewsToken;",
      &g_NewsToken_create);

  jobject ret =
      env->CallStaticObjectMethod(NewsToken_clazz(env),
          method_id, token.obj(), expires);
  jni_generator::CheckException(env);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

// Step 3: RegisterNatives.

}  // namespace opera

#endif  // com_opera_android_dashboard_newsfeed_auth_NewsToken_JNI
