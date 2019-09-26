// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/content_uri_utils.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/platform_file.h"
#include "jni/ContentUriUtils_jni.h"

using base::android::ConvertUTF8ToJavaString;

namespace base {

bool RegisterContentUriUtils(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

bool ContentUriExists(const FilePath& content_uri) {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_uri =
      ConvertUTF8ToJavaString(env, content_uri.value());
  return Java_ContentUriUtils_contentUriExists(
      env, base::android::GetApplicationContext(), j_uri.obj());
}

int OpenContentUriForRead(const FilePath& content_uri) {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> j_uri =
      ConvertUTF8ToJavaString(env, content_uri.value());
  jint fd = Java_ContentUriUtils_openContentUriForRead(
      env, base::android::GetApplicationContext(), j_uri.obj());
  if (fd < 0)
    return base::kInvalidPlatformFileValue;
  return fd;
}

}  // namespace base
