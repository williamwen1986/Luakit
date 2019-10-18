// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/build_info.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "jni/BuildInfo_jni.h"

namespace {

// The caller takes ownership of the returned const char*.
const char* StrDupJString(const base::android::JavaRef<jstring>& java_string) {
  std::string str = ConvertJavaStringToUTF8(java_string);
  return strdup(str.c_str());
}

}  // namespace

namespace base {
namespace android {

struct BuildInfoSingletonTraits {
  static BuildInfo* New() {
    return new BuildInfo(AttachCurrentThread());
  }

  static void Delete(BuildInfo* x) {
    // We're leaking this type, see kRegisterAtExit.
    NOTREACHED();
  }

  static const bool kRegisterAtExit = false;
  static const bool kAllowedToAccessOnNonjoinableThread = true;
};

BuildInfo::BuildInfo(JNIEnv* env)
    : language_(StrDupJString(Java_BuildInfo_getLanguage(env, GetApplicationContext()))),
      device_(StrDupJString(Java_BuildInfo_getDevice(env))),
      model_(StrDupJString(Java_BuildInfo_getDeviceModel(env))),
      brand_(StrDupJString(Java_BuildInfo_getBrand(env))),
      android_build_id_(StrDupJString(Java_BuildInfo_getAndroidBuildId(env))),
      android_build_fp_(StrDupJString(
          Java_BuildInfo_getAndroidBuildFingerprint(env))),
      package_version_code_(StrDupJString(Java_BuildInfo_getPackageVersionCode(
          env, GetApplicationContext()))),
      package_version_name_(StrDupJString(Java_BuildInfo_getPackageVersionName(
          env, GetApplicationContext()))),
      package_label_(StrDupJString(Java_BuildInfo_getPackageLabel(
          env, GetApplicationContext()))),
      package_name_(StrDupJString(Java_BuildInfo_getPackageName(
          env, GetApplicationContext()))),
      sdk_int_(Java_BuildInfo_getSdkInt(env)),
      java_exception_info_(NULL) {
}

// static
BuildInfo* BuildInfo::GetInstance() {
  return Singleton<BuildInfo, BuildInfoSingletonTraits >::get();
}

void BuildInfo::set_java_exception_info(const std::string& info) {
  DCHECK(!java_exception_info_) << "info should be set only once.";
  java_exception_info_ = strndup(info.c_str(), 1024);
}

// static
bool BuildInfo::RegisterBindings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace android
}  // namespace base
