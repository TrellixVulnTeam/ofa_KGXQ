// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/opera/android/library_manager/LibraryManager

#ifndef com_opera_android_library_manager_LibraryManager_JNI
#define com_opera_android_library_manager_LibraryManager_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kLibraryManagerClassPath[] =
    "com/opera/android/library_manager/LibraryManager";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_LibraryManager_clazz __attribute__((unused)) = 0;
#define LibraryManager_clazz(env) base::android::LazyGetClass(env, kLibraryManagerClassPath, &g_LibraryManager_clazz)

}  // namespace

// Step 2: method stubs.

static base::subtle::AtomicWord g_LibraryManager_findMiniLibrary = 0;
static base::android::ScopedJavaLocalRef<jstring>
    Java_LibraryManager_findMiniLibrary(JNIEnv* env) {
  CHECK_CLAZZ(env, LibraryManager_clazz(env),
      LibraryManager_clazz(env), NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, LibraryManager_clazz(env),
      "findMiniLibrary",

"("
")"
"Ljava/lang/String;",
      &g_LibraryManager_findMiniLibrary);

  jstring ret =
static_cast<jstring>(env->CallStaticObjectMethod(LibraryManager_clazz(env),
          method_id));
  jni_generator::CheckException(env);
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

// Step 3: RegisterNatives.

#endif  // com_opera_android_library_manager_LibraryManager_JNI
