// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     org/chromium/components/gcm_driver/GCMDriver

#ifndef org_chromium_components_gcm_driver_GCMDriver_JNI
#define org_chromium_components_gcm_driver_GCMDriver_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kGCMDriverClassPath[] =
    "org/chromium/components/gcm_driver/GCMDriver";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_GCMDriver_clazz __attribute__((unused)) = 0;
#define GCMDriver_clazz(env) base::android::LazyGetClass(env, kGCMDriverClassPath, &g_GCMDriver_clazz)

}  // namespace

namespace gcm {

// Step 2: method stubs.
JNI_GENERATOR_EXPORT void
    Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnRegisterFinished(JNIEnv*
    env, jobject jcaller,
    jlong nativeGCMDriverAndroid,
    jstring appId,
    jstring registrationId,
    jboolean success) {
  GCMDriverAndroid* native =
      reinterpret_cast<GCMDriverAndroid*>(nativeGCMDriverAndroid);
  CHECK_NATIVE_PTR(env, jcaller, native, "OnRegisterFinished");
  return native->OnRegisterFinished(env,
      base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jstring>(env, appId),
      base::android::JavaParamRef<jstring>(env, registrationId), success);
}

JNI_GENERATOR_EXPORT void
    Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnUnregisterFinished(JNIEnv*
    env, jobject jcaller,
    jlong nativeGCMDriverAndroid,
    jstring appId,
    jboolean success) {
  GCMDriverAndroid* native =
      reinterpret_cast<GCMDriverAndroid*>(nativeGCMDriverAndroid);
  CHECK_NATIVE_PTR(env, jcaller, native, "OnUnregisterFinished");
  return native->OnUnregisterFinished(env,
      base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jstring>(env, appId), success);
}

JNI_GENERATOR_EXPORT void
    Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnMessageReceived(JNIEnv*
    env, jobject jcaller,
    jlong nativeGCMDriverAndroid,
    jstring appId,
    jstring senderId,
    jstring collapseKey,
    jbyteArray rawData,
    jobjectArray dataKeysAndValues) {
  GCMDriverAndroid* native =
      reinterpret_cast<GCMDriverAndroid*>(nativeGCMDriverAndroid);
  CHECK_NATIVE_PTR(env, jcaller, native, "OnMessageReceived");
  return native->OnMessageReceived(env,
      base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jstring>(env, appId),
      base::android::JavaParamRef<jstring>(env, senderId),
      base::android::JavaParamRef<jstring>(env, collapseKey),
      base::android::JavaParamRef<jbyteArray>(env, rawData),
      base::android::JavaParamRef<jobjectArray>(env, dataKeysAndValues));
}

static base::subtle::AtomicWord g_GCMDriver_create = 0;
static base::android::ScopedJavaLocalRef<jobject> Java_GCMDriver_create(JNIEnv*
    env, jlong nativeGCMDriverAndroid,
    const base::android::JavaRefOrBare<jobject>& context) {
  CHECK_CLAZZ(env, GCMDriver_clazz(env),
      GCMDriver_clazz(env), NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, GCMDriver_clazz(env),
      "create",

"("
"J"
"Landroid/content/Context;"
")"
"Lorg/chromium/components/gcm_driver/GCMDriver;",
      &g_GCMDriver_create);

  jobject ret =
      env->CallStaticObjectMethod(GCMDriver_clazz(env),
          method_id, nativeGCMDriverAndroid, context.obj());
  jni_generator::CheckException(env);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static base::subtle::AtomicWord g_GCMDriver_destroy = 0;
static void Java_GCMDriver_destroy(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj) {
  CHECK_CLAZZ(env, obj.obj(),
      GCMDriver_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, GCMDriver_clazz(env),
      "destroy",

"("
")"
"V",
      &g_GCMDriver_destroy);

     env->CallVoidMethod(obj.obj(),
          method_id);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_GCMDriver_register = 0;
static void Java_GCMDriver_register(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, const
    base::android::JavaRefOrBare<jstring>& appId,
    const base::android::JavaRefOrBare<jstring>& senderId) {
  CHECK_CLAZZ(env, obj.obj(),
      GCMDriver_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, GCMDriver_clazz(env),
      "register",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_GCMDriver_register);

     env->CallVoidMethod(obj.obj(),
          method_id, appId.obj(), senderId.obj());
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_GCMDriver_unregister = 0;
static void Java_GCMDriver_unregister(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, const
    base::android::JavaRefOrBare<jstring>& appId,
    const base::android::JavaRefOrBare<jstring>& senderId) {
  CHECK_CLAZZ(env, obj.obj(),
      GCMDriver_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, GCMDriver_clazz(env),
      "unregister",

"("
"Ljava/lang/String;"
"Ljava/lang/String;"
")"
"V",
      &g_GCMDriver_unregister);

     env->CallVoidMethod(obj.obj(),
          method_id, appId.obj(), senderId.obj());
  jni_generator::CheckException(env);

}

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsGCMDriver[] = {
    { "nativeOnRegisterFinished",
"("
"J"
"Ljava/lang/String;"
"Ljava/lang/String;"
"Z"
")"
"V",
    reinterpret_cast<void*>(Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnRegisterFinished)
    },
    { "nativeOnUnregisterFinished",
"("
"J"
"Ljava/lang/String;"
"Z"
")"
"V",
    reinterpret_cast<void*>(Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnUnregisterFinished)
    },
    { "nativeOnMessageReceived",
"("
"J"
"Ljava/lang/String;"
"Ljava/lang/String;"
"Ljava/lang/String;"
"[B"
"[Ljava/lang/String;"
")"
"V",
    reinterpret_cast<void*>(Java_org_chromium_components_gcm_1driver_GCMDriver_nativeOnMessageReceived)
    },
};

static bool RegisterNativesImpl(JNIEnv* env) {
  if (base::android::IsManualJniRegistrationDisabled()) return true;

  const int kMethodsGCMDriverSize = arraysize(kMethodsGCMDriver);

  if (env->RegisterNatives(GCMDriver_clazz(env),
                           kMethodsGCMDriver,
                           kMethodsGCMDriverSize) < 0) {
    jni_generator::HandleRegistrationError(
        env, GCMDriver_clazz(env), __FILE__);
    return false;
  }

  return true;
}

}  // namespace gcm

#endif  // org_chromium_components_gcm_driver_GCMDriver_JNI
