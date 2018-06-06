/*
 * JniEnvWrapper.cpp
 *
 *  Created on: 2013-10-15
 *      Author: huangxiaogang
 */

#include "JniEnvWrapper.h"
#include "JniClassMember.h"
#include "base/logging.h"
#include <android/log.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

const char* SIG_INT = "I";
const char* SIG_BOOL = "Z";
const char* SIG_BYTE = "B";
const char* SIG_CHAR = "C";
const char* SIG_SHORT = "S";
const char* SIG_LONG = "J";
const char* SIG_FLOAT = "F";
const char* SIG_DOUBLE = "D";
const char* SIG_STRING = "Ljava/lang/String;";
const char* SIG_INT_ARRAY = "[I";
const char* SIG_BOOL_ARRAY = "[Z";
const char* SIG_BYTE_ARRAY = "[B";
const char* SIG_BYTE_ARRAY_ARRAY = "[[[B";
const char* SIG_CHAR_ARRAY = "[C";
const char* SIG_SHORT_ARRAY = "[S";
const char* SIG_LONG_ARRAY = "[J";
const char* SIG_FLOAT_ARRAY = "[F";
const char* SIG_DOUBLE_ARRAY = "[D";
const char* SIG_STRING_ARRAY =  "[Ljava/lang/String;";

JavaVM* JniEnvWrapper::jvm_ = NULL;

JniEnvWrapper::JniEnvWrapper(JavaVM* jvm) : env_(0), attached_(false) {
	SetJavaVM(jvm);
	Init();
}

JniEnvWrapper::JniEnvWrapper(JNIEnv* env) : env_(env), attached_(false) {

}

JniEnvWrapper::JniEnvWrapper() : env_(0), attached_(false) {
	Init();
}

JniEnvWrapper::JniEnvWrapper(JniEnvWrapper& another) : env_(another.env_) {

}

JniEnvWrapper::~JniEnvWrapper() {
	if (attached_) {
		jvm_->DetachCurrentThread();
	}
	env_ = 0;
}

JniEnvWrapper& JniEnvWrapper::operator=(const JniEnvWrapper &another) {
	env_ = another.env_;
	return *this;
}

JniEnvWrapper::operator JNIEnv* () const {
	return env_;
}

JNIEnv* JniEnvWrapper::operator*() const {
	return env_;
}

JNIEnv* JniEnvWrapper::operator->() const {
	return env_;
}

JavaVM* JniEnvWrapper::GetJavaVM() {
	return jvm_;
}

void JniEnvWrapper::SetJavaVM(JavaVM* jvm) {
	jvm_ = jvm;
}

#define IMPL_GET_TYPE_FIELD(FieldType, FieldSig) \
		jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, FieldSig); \
		DCHECK(fieldid != 0);\
		return env_->Get##FieldType##Field(obj, fieldid);

jint JniEnvWrapper::GetIntField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Int, SIG_INT);
}

jboolean JniEnvWrapper::GetBoolField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Boolean, SIG_BOOL);
}

jbyte JniEnvWrapper::GetByteField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Byte, SIG_BYTE);
}

jchar JniEnvWrapper::GetCharField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Char, SIG_CHAR);
}

jshort JniEnvWrapper::GetShortField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Short, SIG_SHORT);
}

jlong JniEnvWrapper::GetLongField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Long, SIG_LONG);
}

jfloat JniEnvWrapper::GetFloatField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Float, SIG_FLOAT);
}

jdouble JniEnvWrapper::GetDoubleField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_TYPE_FIELD(Double, SIG_DOUBLE);
}

jobject JniEnvWrapper::GetObjectField(jobject obj, const char* classsig, const char* name, const char* sig) {
	jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, sig);
	DCHECK(fieldid != 0);

	return env_->GetObjectField(obj, fieldid);
}

jstring JniEnvWrapper::GetStringField(jobject obj, const char* classsig, const char* name) {
	return (jstring)GetObjectField(obj, classsig, name, SIG_STRING);
}

#define IMPL_GET_STATIC_TYPE_FIELD(FieldType, FieldSig) \
		jfieldID fieldid = JniClassMember::GetInstance()->GetStaticField(env_, obj, classsig, name, FieldSig); \
		DCHECK(fieldid != 0);\
		return env_->GetStatic##FieldType##Field(env_->GetObjectClass(obj), fieldid);

int JniEnvWrapper::GetStaticIntField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Int, SIG_INT);
}

jboolean JniEnvWrapper::GetStaticBoolField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Boolean, SIG_BOOL);
}

jbyte JniEnvWrapper::GetStaticByteField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Byte, SIG_BYTE);
}

jchar JniEnvWrapper::GetStaticCharField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Char, SIG_CHAR);
}

jshort JniEnvWrapper::GetStaticShortField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Short, SIG_SHORT);
}

jlong JniEnvWrapper::GetStaticLongField(jobject obj, const char* classsig, const char* name) {
	IMPL_GET_STATIC_TYPE_FIELD(Long, SIG_LONG);
}

jfloat JniEnvWrapper::GetStaticFloatField(jobject obj, const char* classsig, const char* name){
	IMPL_GET_STATIC_TYPE_FIELD(Float, SIG_FLOAT);
}

jdouble JniEnvWrapper::GetStaticDoubleField(jobject obj, const char* classsig, const char* name){
	IMPL_GET_STATIC_TYPE_FIELD(Double, SIG_DOUBLE);
}

jobject JniEnvWrapper::GetStaticObjectField(jobject obj, const char* classsig, const char* name, const char* sig){
	jfieldID fieldid = JniClassMember::GetInstance()->GetStaticField(env_, obj, classsig, name, sig);
	DCHECK(fieldid != 0);

	return env_->GetStaticObjectField(env_->GetObjectClass(obj), fieldid);
}

jstring JniEnvWrapper::GetStaticStringField(jobject obj, const char* classsig, const char* name){
	return (jstring)GetStaticObjectField(obj, classsig, name, SIG_STRING);
}

#define IMPL_SET_TYPE_FIELD(FieldType, FieldSig) \
	jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, FieldSig);\
	DCHECK(fieldid != 0);\
	env_->Set##FieldType##Field(obj, fieldid, value);

void JniEnvWrapper::SetIntField(jobject obj, const char* classsig, const char* name, jint value) {
	IMPL_SET_TYPE_FIELD(Int, SIG_INT);
}

void JniEnvWrapper::SetBoolField(jobject obj, const char* classsig, const char* name, jboolean value) {
	IMPL_SET_TYPE_FIELD(Boolean, SIG_BOOL);
}

void JniEnvWrapper::SetByteField(jobject obj, const char* classsig, const char* name, jbyte value) {
	IMPL_SET_TYPE_FIELD(Byte, SIG_BYTE);
}

void JniEnvWrapper::SetCharField(jobject obj, const char* classsig, const char* name, jchar value) {
	IMPL_SET_TYPE_FIELD(Char, SIG_CHAR);
}

void JniEnvWrapper::SetShortField(jobject obj, const char* classsig, const char* name, jshort value) {
	IMPL_SET_TYPE_FIELD(Short, SIG_SHORT);
}

void JniEnvWrapper::SetLongField(jobject obj, const char* classsig, const char* name, jlong value) {
	jfieldID test_field = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, SIG_LONG);\
	if (test_field == 0) {
		__android_log_print(ANDROID_LOG_INFO, "JniEnvWrapper", "test_field = 0");
	}

	IMPL_SET_TYPE_FIELD(Long, SIG_LONG);
}

void JniEnvWrapper::SetFloatField(jobject obj, const char* classsig, const char* name, jfloat value) {
	IMPL_SET_TYPE_FIELD(Float, SIG_FLOAT);
}

void JniEnvWrapper::SetDoubleField(jobject obj, const char* classsig, const char* name, jdouble value) {
	IMPL_SET_TYPE_FIELD(Double, SIG_DOUBLE);
}

void JniEnvWrapper::SetObjectField(jobject obj, const char* classsig, const char* name, const char* sig, jobject value){
	jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, sig);
	DCHECK(fieldid != 0);

	env_->SetObjectField(obj, fieldid, value);
}

void JniEnvWrapper::SetStringField(jobject obj, const char* classsig, const char* name, jstring value) {
	SetObjectField(obj, classsig, name, SIG_STRING, (jobject)value);
}

#define IMPL_SET_STATIC_TYPE_FIELD(FieldType, FieldSig) \
	jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig,name, FieldSig);\
	DCHECK(fieldid != 0);\
	env_->SetStatic##FieldType##Field(env_->GetObjectClass(obj), fieldid, value);

void JniEnvWrapper::SetStaticIntField(jobject obj, const char* classsig, const char* name, jint value) {
	IMPL_SET_STATIC_TYPE_FIELD(Int, SIG_INT);
}

void JniEnvWrapper::SetStaticBoolField(jobject obj, const char* classsig, const char* name, jboolean value) {
	IMPL_SET_STATIC_TYPE_FIELD(Boolean, SIG_BOOL);
}

void JniEnvWrapper::SetStaticByteField(jobject obj, const char* classsig, const char* name, jbyte value) {
	IMPL_SET_STATIC_TYPE_FIELD(Byte, SIG_BYTE);
}

void JniEnvWrapper::SetStaticCharField(jobject obj, const char* classsig, const char* name, jchar value) {
	IMPL_SET_STATIC_TYPE_FIELD(Char, SIG_CHAR);
}

void JniEnvWrapper::SetStaticShortField(jobject obj, const char* classsig, const char* name, jshort value) {
	IMPL_SET_STATIC_TYPE_FIELD(Short, SIG_SHORT);
}

void JniEnvWrapper::SetStaticLongField(jobject obj, const char* classsig, const char* name, jlong value) {
	IMPL_SET_STATIC_TYPE_FIELD(Long, SIG_LONG);
}

void JniEnvWrapper::SetStaticFloatField(jobject obj, const char* classsig, const char* name, jfloat value) {
	IMPL_SET_STATIC_TYPE_FIELD(Float, SIG_FLOAT);
}

void JniEnvWrapper::SetStaticDoubleField(jobject obj, const char* classsig, const char* name, jdouble value) {
	IMPL_SET_STATIC_TYPE_FIELD(Double, SIG_DOUBLE);
}

void JniEnvWrapper::SetStaticObjectField(jobject obj, const char* classsig, const char* name, const char* sig, jobject value){
	jfieldID fieldid = JniClassMember::GetInstance()->GetField(env_, obj, classsig, name, sig);
	DCHECK(fieldid != 0);

	env_->SetStaticObjectField(env_->GetObjectClass(obj), fieldid, value);
}

void JniEnvWrapper::SetStaticStringField(jobject obj, const char* classsig, const char* name, jstring value) {
	SetStaticObjectField(env_->GetObjectClass(obj), classsig, name, SIG_STRING, (jobject)value);
}

jobjectArray JniEnvWrapper::GetObjectArray(jobject obj, const char* classsig, const char* name, const char* sig){
	return (jobjectArray)GetObjectField(obj, classsig, name, sig);
}

jobjectArray JniEnvWrapper::GetStringArray(jobject obj, const char* classsig, const char* name) {
	return GetObjectArray(obj, classsig, name, SIG_STRING_ARRAY);
}

jbooleanArray JniEnvWrapper::GetBoolArray(jobject obj, const char* classsig, const char* name){
	return (jbooleanArray)GetObjectArray(obj, classsig, name, SIG_BOOL_ARRAY);
}

jbyteArray JniEnvWrapper::GetByteArray(jobject obj, const char* classsig, const char* name){
	return (jbyteArray)GetObjectArray(obj, classsig, name, SIG_BYTE_ARRAY);
}

jcharArray JniEnvWrapper::GetCharArray(jobject obj, const char* classsig, const char* name){
	return (jcharArray)GetObjectArray(obj, classsig, name, SIG_CHAR_ARRAY);
}

jshortArray JniEnvWrapper::GetShortArray(jobject obj, const char* classsig, const char* name){
	return (jshortArray)GetObjectArray(obj, classsig, name, SIG_SHORT_ARRAY);
}

jintArray JniEnvWrapper::GetIntArray(jobject obj, const char* classsig, const char* name){
	return (jintArray)GetObjectArray(obj, classsig, name, SIG_INT_ARRAY);
}

jlongArray JniEnvWrapper::GetLongArray(jobject obj, const char* classsig, const char* name){
	return (jlongArray)GetObjectArray(obj, classsig, name, SIG_LONG_ARRAY);
}

jfloatArray JniEnvWrapper::GetFloatArray(jobject obj, const char* classsig, const char* name){
	return (jfloatArray)GetObjectArray(obj, classsig, name, SIG_FLOAT_ARRAY);
}

jdoubleArray JniEnvWrapper::GetDoubleArray(jobject obj, const char* classsig, const char* name){
	return (jdoubleArray)GetObjectArray(obj, classsig, name, SIG_DOUBLE_ARRAY);
}


void JniEnvWrapper::SetObjectArray(jobject obj, const char* classsig, const char* name, const char* sig, jobjectArray value){
	SetObjectField(obj, classsig, name, sig, value);
}

void JniEnvWrapper::SetStringArray(jobject obj, const char* classsig, const char* name, jobjectArray value) {
	SetObjectArray(obj, classsig, name, SIG_STRING_ARRAY, value);
}

void JniEnvWrapper::SetBoolArray(jobject obj, const char* classsig, const char* name, jbooleanArray value){
	SetObjectField(obj, classsig, name, SIG_BOOL_ARRAY, value);
}

void JniEnvWrapper::SetByteArray(jobject obj, const char* classsig, const char* name, jbyteArray value){
	SetObjectField(obj, classsig, name, SIG_BYTE_ARRAY, value);
}

void JniEnvWrapper::SetByteArrayArray(jobject obj, const char* classsig, const char* name, jobjectArray value){
	SetObjectField(obj, classsig, name, SIG_BYTE_ARRAY_ARRAY, value);
}

void JniEnvWrapper::SetCharArray(jobject obj, const char* classsig, const char* name, jcharArray value){
	SetObjectField(obj, classsig, name, SIG_CHAR_ARRAY, value);
}

void JniEnvWrapper::SetShortArray(jobject obj, const char* classsig, const char* name, jshortArray value){
	SetObjectField(obj, classsig, name, SIG_SHORT_ARRAY, value);
}

void JniEnvWrapper::SetIntArray(jobject obj, const char* classsig, const char* name, jintArray value){
	SetObjectField(obj, classsig, name, SIG_INT_ARRAY, value);
}

void JniEnvWrapper::SetLongArray(jobject obj, const char* classsig, const char* name, jlongArray value){
	SetObjectField(obj, classsig, name, SIG_LONG_ARRAY, value);
}

void JniEnvWrapper::SetFloatArray(jobject obj, const char* classsig, const char* name, jfloatArray value){
	SetObjectField(obj, classsig, name, SIG_FLOAT_ARRAY, value);
}

void JniEnvWrapper::SetDoubleArray(jobject obj, const char* classsig, const char* name, jdoubleArray value){
	SetObjectField(obj, classsig, name, SIG_DOUBLE_ARRAY, value);
}

// method implement
#define CALL_TYPE_METHOD_IMPL(returnType, methodType) \
		jmethodID methodid = JniClassMember::GetInstance()->GetMethod(env_, obj, classsig, name, sig); \
		DCHECK(methodid != 0); \
		va_list args; \
		va_start (args, sig); \
		returnType result = env_->Call##methodType##MethodV(obj, methodid, args);\
		va_end (args);\
		return result;

jobject JniEnvWrapper::CallObjectMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jobject, Object);
}

jboolean JniEnvWrapper::CallBooleanMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jboolean, Boolean);
}

jbyte JniEnvWrapper::CallByteMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jbyte, Byte);
}

jchar JniEnvWrapper::CallCharMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jchar, Char);
}

jshort JniEnvWrapper::CallShortMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jshort, Short);
}

jint JniEnvWrapper::CallIntMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jint, Int);
}

jlong JniEnvWrapper::CallLongMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jlong, Long);
}

jfloat JniEnvWrapper::CallFloatMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jfloat, Float);
}

jdouble JniEnvWrapper::CallDoubleMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_TYPE_METHOD_IMPL(jdouble, Double);
}

void JniEnvWrapper::CallVoidMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	// LOG(INFO) << "classsig = " << classsig << ", name = " << name << ", sig = " << sig;
	jmethodID methodid = JniClassMember::GetInstance()->GetMethod(env_, obj, classsig, name, sig);
	if (methodid == 0) {
        LOG(ERROR) << "methodid = 0. classsig = " << classsig << ", name = " << name << ", sig = " << sig;
	}
	
	DCHECK(methodid != 0);
	va_list args;
	va_start (args, sig);
	env_->CallVoidMethodV(obj, methodid, args);
	va_end (args);
}

#define CALL_STATIC_TYPE_METHOD_IMPL(returnType, methodType) \
		jmethodID methodid = JniClassMember::GetInstance()->GetStaticMethod(env_, obj, classsig, name, sig); \
		DCHECK(methodid != 0); \
		va_list args; \
		va_start (args, sig); \
		returnType result = env_->CallStatic##methodType##MethodV(env_->GetObjectClass(obj), methodid, args);\
		va_end (args);\
		return result;

jobject JniEnvWrapper::CallStaticObjectMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...) {
	CALL_STATIC_TYPE_METHOD_IMPL(jobject, Object);
}

jboolean JniEnvWrapper::CallStaticBooleanMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jboolean, Boolean);
}

jbyte JniEnvWrapper::CallStaticByteMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jbyte, Byte);
}

jchar JniEnvWrapper::CallStaticCharMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jchar, Char);
}

jshort JniEnvWrapper::CallStaticShortMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jshort, Short);
}

jint JniEnvWrapper::CallStaticIntMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jint, Int);
}

jlong JniEnvWrapper::CallStaticLongMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jlong, Long);
}

jfloat JniEnvWrapper::CallStaticFloatMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jfloat, Float);
}

jdouble JniEnvWrapper::CallStaticDoubleMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	CALL_STATIC_TYPE_METHOD_IMPL(jdouble, Double);
}

void JniEnvWrapper::CallStaticVoidMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...){
	jmethodID methodid = JniClassMember::GetInstance()->GetStaticMethod(env_, obj, classsig, name, sig);
	DCHECK(methodid != 0);
	va_list args;
	va_start (args, sig);
	env_->CallStaticVoidMethodV(env_->GetObjectClass(obj), methodid, args);
	va_end (args);
}

#define CALL_CLASS_STATIC_TYPE_METHOD_IMPL(returnType, methodType) \
		jclass classid = JniClassMember::GetInstance()->GetClass(env_, classsig); \
		DCHECK(classid != 0); \
		jmethodID methodid = JniClassMember::GetInstance()->GetStaticMethod(env_, classid, classsig, name, sig); \
		DCHECK(methodid != 0); \
		va_list args; \
		va_start (args, sig); \
		returnType result = env_->CallStatic##methodType##MethodV(classid, methodid, args);\
		va_end (args);\
		return result;

jobject 	JniEnvWrapper::CallStaticObjectMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jobject, Object);
}

jboolean 	JniEnvWrapper::CallStaticBooleanMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jboolean, Boolean);
}

jbyte 		JniEnvWrapper::CallStaticByteMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jbyte, Byte);
}

jchar 		JniEnvWrapper::CallStaticCharMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jchar, Char);
}

jshort 		JniEnvWrapper::CallStaticShortMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jshort, Short);
}

jint 		JniEnvWrapper::CallStaticIntMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jint, Int);
}

jlong 		JniEnvWrapper::CallStaticLongMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jlong, Long);
}

jfloat 		JniEnvWrapper::CallStaticFloatMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jfloat, Float);
}

jdouble 	JniEnvWrapper::CallStaticDoubleMethod(const char* classsig, const char* name, const char* sig, ...){
	CALL_CLASS_STATIC_TYPE_METHOD_IMPL(jdouble, Double);
}

void 		JniEnvWrapper::CallStaticVoidMethod(const char* classsig, const char* name, const char* sig, ...){
	jclass classid = JniClassMember::GetInstance()->GetClass(env_, classsig);
	DCHECK(classid != 0);
	jmethodID methodid = JniClassMember::GetInstance()->GetStaticMethod(env_, classid, classsig, name, sig);
	DCHECK(methodid != 0);
	va_list args;
	va_start (args, sig);
	env_->CallStaticVoidMethodV(classid, methodid, args);
	va_end (args);\
}

jobject JniEnvWrapper::NewObject(const char* classSig, const char* constructorSig, ...){
	jclass cls = 0;
	jmethodID methodid = 0;
	JniClassMember::GetInstance()->GetConstructor(env_, classSig, constructorSig, cls, methodid);

	DCHECK(methodid != 0 && cls != 0);

    va_list args;
    va_start(args, constructorSig);
    jobject result = env_->NewObjectV(cls, methodid, args);
    va_end(args);

    return result;
}
/*
jstring JniEnvWrapper::NewJstring(const char* text, int len) {
	__android_log_print(ANDROID_LOG_INFO, "JniEnvWrapper", "NewJstring : text = %s len = %d", text, len);
	jcharArray elemArr = env_->NewCharArray(len);
	env_->SetCharArrayRegion(elemArr, 0, len, (const jchar*)text);
	jstring result = (jstring)NewObject("java/lang/String", "([C)V", elemArr);

	__android_log_print(ANDROID_LOG_INFO, "JniEnvWrapper", "NewJstring : result = %x", result);
	env_->DeleteLocalRef(elemArr);
	return result;
}*/

jobjectArray JniEnvWrapper::NewObjectArray(const char* classSig, int elementCount, jobject initialElement) {
	jclass cls = JniClassMember::GetInstance()->GetClass(env_, classSig);

	DCHECK(cls != 0);

	return env_->NewObjectArray(elementCount, cls, initialElement);
}

void JniEnvWrapper::Init() {
	int status = jvm_->GetEnv((void**)&env_, JNI_VERSION_1_6);
	if (status < 0) {
		status = jvm_->AttachCurrentThread(&env_, 0);
		if (status < 0) {
			// Failed to get JNIEnv object, so we are not able to throw an exception.
			__android_log_assert("assert", "Native-JNIUtils", "Failed to get JNIEnv object, so we are not able to throw an exception.");
		} else {
			attached_ = true;
		}
	}
}
