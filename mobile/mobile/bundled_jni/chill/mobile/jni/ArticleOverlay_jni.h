// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     base/android/jni_generator/jni_generator.py
// For
//     com/opera/android/articles/ArticleOverlay

#ifndef com_opera_android_articles_ArticleOverlay_JNI
#define com_opera_android_articles_ArticleOverlay_JNI

#include <jni.h>

#include "../../../../../../../../../../../../../../../base/android/jni_generator/jni_generator_helper.h"

#include "base/android/jni_int_wrapper.h"

// Step 1: forward declarations.
namespace {
const char kArticleOverlayClassPath[] =
    "com/opera/android/articles/ArticleOverlay";
// Leaking this jclass as we cannot use LazyInstance from some threads.
base::subtle::AtomicWord g_ArticleOverlay_clazz __attribute__((unused)) = 0;
#define ArticleOverlay_clazz(env) base::android::LazyGetClass(env, kArticleOverlayClassPath, &g_ArticleOverlay_clazz)

}  // namespace

namespace opera {

// Step 2: method stubs.

static base::android::ScopedJavaLocalRef<jobject> FromWebContents(JNIEnv* env,
    const base::android::JavaParamRef<jclass>& jcaller,
    const base::android::JavaParamRef<jobject>& webcontents);

JNI_GENERATOR_EXPORT jobject
    Java_com_opera_android_articles_ArticleOverlay_nativeFromWebContents(JNIEnv*
    env, jclass jcaller,
    jobject webcontents) {
  return FromWebContents(env, base::android::JavaParamRef<jclass>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, webcontents)).Release();
}

JNI_GENERATOR_EXPORT void
    Java_com_opera_android_articles_ArticleOverlay_nativeOpenArticle(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay,
    jobject article,
    jboolean transcodedMode) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "OpenArticle");
  return native->OpenArticle(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, article),
      transcodedMode);
}

JNI_GENERATOR_EXPORT void
    Java_com_opera_android_articles_ArticleOverlay_nativeToggleReadingMode(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "ToggleReadingMode");
  return native->ToggleReadingMode(env,
      base::android::JavaParamRef<jobject>(env, jcaller));
}

static jlong Init(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller,
    const base::android::JavaParamRef<jobject>& webContents,
    const base::android::JavaParamRef<jstring>& transcoderHost);

JNI_GENERATOR_EXPORT jlong
    Java_com_opera_android_articles_ArticleOverlay_nativeInit(JNIEnv* env,
    jobject jcaller,
    jobject webContents,
    jstring transcoderHost) {
  return Init(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, webContents),
      base::android::JavaParamRef<jstring>(env, transcoderHost));
}

JNI_GENERATOR_EXPORT void
    Java_com_opera_android_articles_ArticleOverlay_nativeActivateReadingMode(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "ActivateReadingMode");
  return native->ActivateReadingMode(env,
      base::android::JavaParamRef<jobject>(env, jcaller));
}

JNI_GENERATOR_EXPORT void
    Java_com_opera_android_articles_ArticleOverlay_nativeReloadArticle(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "ReloadArticle");
  return native->ReloadArticle(env, base::android::JavaParamRef<jobject>(env,
      jcaller));
}

JNI_GENERATOR_EXPORT jboolean
    Java_com_opera_android_articles_ArticleOverlay_nativeIsShowingArticleForVisibleEntry(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "IsShowingArticleForVisibleEntry",
      false);
  return native->IsShowingArticleForVisibleEntry(env,
      base::android::JavaParamRef<jobject>(env, jcaller));
}

JNI_GENERATOR_EXPORT jobject
    Java_com_opera_android_articles_ArticleOverlay_nativeGetVisibleArticle(JNIEnv*
    env, jobject jcaller,
    jlong nativeArticleOverlay) {
  ArticleOverlay* native =
      reinterpret_cast<ArticleOverlay*>(nativeArticleOverlay);
  CHECK_NATIVE_PTR(env, jcaller, native, "GetVisibleArticle", NULL);
  return native->GetVisibleArticle(env,
      base::android::JavaParamRef<jobject>(env, jcaller)).Release();
}

static base::subtle::AtomicWord g_ArticleOverlay_updateReadingModeState = 0;
static void Java_ArticleOverlay_updateReadingModeState(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, jboolean enabled,
    jboolean isReadingMode) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "updateReadingModeState",

"("
"Z"
"Z"
")"
"V",
      &g_ArticleOverlay_updateReadingModeState);

     env->CallVoidMethod(obj.obj(),
          method_id, enabled, isReadingMode);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_ArticleOverlay_attachContentViewCore = 0;
static void Java_ArticleOverlay_attachContentViewCore(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, const
    base::android::JavaRefOrBare<jobject>& cvc,
    jboolean isReadingMode) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "attachContentViewCore",

"("
"Lorg/chromium/content/browser/ContentViewCore;"
"Z"
")"
"V",
      &g_ArticleOverlay_attachContentViewCore);

     env->CallVoidMethod(obj.obj(),
          method_id, cvc.obj(), isReadingMode);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_ArticleOverlay_onLoaded = 0;
static void Java_ArticleOverlay_onLoaded(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "onLoaded",

"("
")"
"V",
      &g_ArticleOverlay_onLoaded);

     env->CallVoidMethod(obj.obj(),
          method_id);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_ArticleOverlay_onReadingModeChanged = 0;
static void Java_ArticleOverlay_onReadingModeChanged(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, jboolean readingModeActive) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "onReadingModeChanged",

"("
"Z"
")"
"V",
      &g_ArticleOverlay_onReadingModeChanged);

     env->CallVoidMethod(obj.obj(),
          method_id, readingModeActive);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_ArticleOverlay_setActive = 0;
static void Java_ArticleOverlay_setActive(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, jboolean active) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "setActive",

"("
"Z"
")"
"V",
      &g_ArticleOverlay_setActive);

     env->CallVoidMethod(obj.obj(),
          method_id, active);
  jni_generator::CheckException(env);

}

static base::subtle::AtomicWord g_ArticleOverlay_loadingFailed = 0;
static void Java_ArticleOverlay_loadingFailed(JNIEnv* env, const
    base::android::JavaRefOrBare<jobject>& obj, const
    base::android::JavaRefOrBare<jstring>& errorDescription) {
  CHECK_CLAZZ(env, obj.obj(),
      ArticleOverlay_clazz(env));
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_INSTANCE>(
      env, ArticleOverlay_clazz(env),
      "loadingFailed",

"("
"Ljava/lang/String;"
")"
"V",
      &g_ArticleOverlay_loadingFailed);

     env->CallVoidMethod(obj.obj(),
          method_id, errorDescription.obj());
  jni_generator::CheckException(env);

}

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsArticleOverlay[] = {
    { "nativeFromWebContents",
"("
"Lorg/chromium/content_public/browser/WebContents;"
")"
"Lcom/opera/android/articles/ArticleOverlay;",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeFromWebContents)
    },
    { "nativeOpenArticle",
"("
"J"
"Lcom/opera/android/dashboard/newsfeed/data/Article;"
"Z"
")"
"V",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeOpenArticle)
    },
    { "nativeToggleReadingMode",
"("
"J"
")"
"V",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeToggleReadingMode)
    },
    { "nativeInit",
"("
"Lorg/chromium/content_public/browser/WebContents;"
"Ljava/lang/String;"
")"
"J",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeInit)
    },
    { "nativeActivateReadingMode",
"("
"J"
")"
"V",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeActivateReadingMode)
    },
    { "nativeReloadArticle",
"("
"J"
")"
"V",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeReloadArticle)
    },
    { "nativeIsShowingArticleForVisibleEntry",
"("
"J"
")"
"Z",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeIsShowingArticleForVisibleEntry)
    },
    { "nativeGetVisibleArticle",
"("
"J"
")"
"Lcom/opera/android/dashboard/newsfeed/data/Article;",
    reinterpret_cast<void*>(Java_com_opera_android_articles_ArticleOverlay_nativeGetVisibleArticle)
    },
};

static bool RegisterNativesImpl(JNIEnv* env) {
  if (base::android::IsManualJniRegistrationDisabled()) return true;

  const int kMethodsArticleOverlaySize = arraysize(kMethodsArticleOverlay);

  if (env->RegisterNatives(ArticleOverlay_clazz(env),
                           kMethodsArticleOverlay,
                           kMethodsArticleOverlaySize) < 0) {
    jni_generator::HandleRegistrationError(
        env, ArticleOverlay_clazz(env), __FILE__);
    return false;
  }

  return true;
}

}  // namespace opera

#endif  // com_opera_android_articles_ArticleOverlay_JNI