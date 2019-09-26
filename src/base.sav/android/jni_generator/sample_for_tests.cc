// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_generator/sample_for_tests.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "jni/SampleForTests_jni.h"


using base::android::AttachCurrentThread;
using base::android::ScopedJavaLocalRef;

namespace base {
namespace android {

jdouble CPPClass::InnerClass::MethodOtherP0(JNIEnv* env, jobject obj) {
  return 0.0;
}

CPPClass::CPPClass() {
}

CPPClass::~CPPClass() {
}

void CPPClass::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

jint CPPClass::Method(JNIEnv* env, jobject obj) {
  return 0;
}

void CPPClass::AddStructB(JNIEnv* env, jobject obj, jobject structb) {
  long key = Java_InnerStructB_getKey(env, structb);
  std::string value = ConvertJavaStringToUTF8(
      env, Java_InnerStructB_getValue(env, structb).obj());
  map_[key] = value;
}

void CPPClass::IterateAndDoSomethingWithStructB(JNIEnv* env, jobject obj) {
  // Iterate over the elements and do something with them.
  for (std::map<long, std::string>::const_iterator it = map_.begin();
       it != map_.end(); ++it) {
    long key = it->first;
    std::string value = it->second;
  }
  map_.clear();
}

base::android::ScopedJavaLocalRef<jstring> CPPClass::ReturnAString(
    JNIEnv* env, jobject obj) {
  base::android::ScopedJavaLocalRef<jstring> ret = ConvertUTF8ToJavaString(
      env, "test");
  return ret;
}

// Static free functions declared and called directly from java.
static jint Init(JNIEnv* env, jobject obj, jstring param) {
  return 0;
}

static jdouble GetDoubleFunction(JNIEnv*, jobject) {
  return 0;
}

static jfloat GetFloatFunction(JNIEnv*, jclass) {
  return 0;
}

static void SetNonPODDatatype(JNIEnv*, jobject, jobject) {
}

static jobject GetNonPODDatatype(JNIEnv*, jobject) {
  return NULL;
}

static jint InnerFunction(JNIEnv*, jclass) {
  return 0;
}

} // namespace android
} // namespace base

int main() {
  // On a regular application, you'd call AttachCurrentThread(). This sample is
  // not yet linking with all the libraries.
  JNIEnv* env = /* AttachCurrentThread() */ NULL;

  // This is how you call a java static method from C++.
  bool foo = base::android::Java_SampleForTests_staticJavaMethod(env);

  // This is how you call a java method from C++. Note that you must have
  // obtained the jobject somehow.
  jobject my_java_object = NULL;
  int bar = base::android::Java_SampleForTests_javaMethod(
      env, my_java_object, 1, 2);

  for (int i = 0; i < 10; ++i) {
    // Creates a "struct" that will then be used by the java side.
    ScopedJavaLocalRef<jobject> struct_a =
        base::android::Java_InnerStructA_create(
            env, 0, 1,
            base::android::ConvertUTF8ToJavaString(env, "test").obj());
    base::android::Java_SampleForTests_addStructA(
        env, my_java_object, struct_a.obj());
  }
  base::android::Java_SampleForTests_iterateAndDoSomething(env, my_java_object);
  return 0;
}
