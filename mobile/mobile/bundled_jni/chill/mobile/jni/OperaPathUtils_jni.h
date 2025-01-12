// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/opera/android/utilities/OperaPathUtils

#ifndef com_opera_android_utilities_OperaPathUtils_JNI
#define com_opera_android_utilities_OperaPathUtils_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kOperaPathUtilsClassPath[] =
    "com/opera/android/utilities/OperaPathUtils";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_OperaPathUtils_clazz __attribute__((unused)) = 0;
#define OperaPathUtils_clazz(env) base::android::LazyGetClass(env, kOperaPathUtilsClassPath, &g_OperaPathUtils_clazz)

}  // namespace

// Step 2: method stubs.

static base::subtle::AtomicWord g_OperaPathUtils_getBreamDataStoreDirectory = 0;
static base::android::ScopedJavaLocalRef<jstring>
    Java_OperaPathUtils_getBreamDataStoreDirectory(JNIEnv* env) {
  CHECK_CLAZZ(env, OperaPathUtils_clazz(env),
      OperaPathUtils_clazz(env), NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, OperaPathUtils_clazz(env),
      "getBreamDataStoreDirectory",

"("
")"
"Ljava/lang/String;",
      &g_OperaPathUtils_getBreamDataStoreDirectory);

  jstring ret =
static_cast<jstring>(env->CallStaticObjectMethod(OperaPathUtils_clazz(env),
          method_id));
  jni_generator::CheckException(env);
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

// Step 3: RegisterNatives.

#endif  // com_opera_android_utilities_OperaPathUtils_JNI
