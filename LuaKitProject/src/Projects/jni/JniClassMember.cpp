/*
 * JniClassMember.cpp
 *
 *  Created on: 2013-10-15
 *      Author: huangxiaogang
 */

#include "JniClassMember.h"
#include "base/logging.h"
#include <android/log.h>

JniClassMember*JniClassMember::m_pInstance = NULL;

JniClassMember* JniClassMember::GetInstance() {
	if (m_pInstance == NULL) {
		m_pInstance = new JniClassMember();
	}

	return m_pInstance;
}

JniClassMember::JniClassMember() {

}

JniClassMember::~JniClassMember() {
}

jfieldID JniClassMember::GetStaticField(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig){
	DCHECK(env != 0 && cls != 0 && name != 0 && sig != 0);

	std::string key = MakeKey(classsig, name, sig);
	jfieldID fieldid = QueryFieldID(key);
	if (fieldid == 0) {
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetStaticField(%s) : first get fieldid", key.c_str());

		fieldid = env->GetStaticFieldID(cls, name, sig);
		if (fieldid != 0) {
			InsertFieldID(key, fieldid);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetStaticField(%s) : failed", key.c_str());
		}
	}

	return fieldid;
}

jfieldID JniClassMember::GetStaticField(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig) {
	DCHECK(env != 0 && obj != 0);
	jclass cls = env->GetObjectClass(obj);

	return GetStaticField(env, cls, classsig, name, sig);
}

jfieldID JniClassMember::GetField(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig) {
	DCHECK(env != 0 && obj != 0);

	std::string key = MakeKey(classsig, name, sig);
	jfieldID fieldid = QueryFieldID(key);
	// __android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetField(%s)", key.c_str());

	if (fieldid == 0) {
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetField(%s) : first get fieldid", key.c_str());
		jclass cls = env->GetObjectClass(obj);
		fieldid = env->GetFieldID(cls, name, sig);
		if (fieldid != 0) {
			InsertFieldID(key, fieldid);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetField(%s) : failed", key.c_str());
		}
	}

	return fieldid;
}

jmethodID JniClassMember::GetStaticMethod(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig){
	DCHECK(env != 0 && cls != 0 && name != 0 && sig != 0);

	std::string key = MakeKey(classsig, name, sig);
	jmethodID methodid = QueryMethodID(key);

	if (methodid == 0) {
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetStaticMethod(%s) : first get methodid", key.c_str());
		methodid = env->GetStaticMethodID(cls, name, sig);
		if (methodid != 0) {
			InsertMethodID(key, methodid);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetStaticMethod(%s) : failed", key.c_str());
		}
	}

	return methodid;
}

jmethodID JniClassMember::GetStaticMethod(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig) {
	DCHECK(env != 0 && obj != 0 && name != 0 && sig != 0);
	jclass cls = env->GetObjectClass(obj);

	return GetStaticMethod(env, cls, classsig, name, sig);
}

jmethodID JniClassMember::GetMethod(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig) {
	DCHECK(env != 0 && obj != 0 && name != 0 && sig != 0);

	jclass cls = env->GetObjectClass(obj);
	return GetMethod(env, cls, classsig, name, sig);
}

jmethodID JniClassMember::GetMethod(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig){
	DCHECK(env != 0 && cls != 0 && name != 0 && sig != 0);

	std::string key = MakeKey(classsig, name, sig);
	//android 2.3.3中，不能缓存非静态方法的methodId
	//jmethodID methodid = QueryMethodID(key);
	jmethodID methodid = 0;
//	__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetMethod(%s)", key.c_str());

	if (methodid == 0) {
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetMethod(%s) : first get methodid", key.c_str());
		methodid = env->GetMethodID(cls, name, sig);
		if (methodid != 0) {
			InsertMethodID(key, methodid);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetMethod(%s) : failed", key.c_str());
		}
	}

	return methodid;
}

void JniClassMember::GetConstructor(JNIEnv* env, const char* classSig, const char* constructorSig, jclass& cls, jmethodID& methodid) {
	DCHECK(env != 0 && classSig != 0 && constructorSig != 0);

	cls = GetClass(env, classSig);

	DCHECK(cls != 0);

	std::string key(classSig);
	key.append(" - <init> - ").append(constructorSig);

	methodid = QueryMethodID(key);

	if (methodid == 0){
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetConstructor(%s) : first get Constructor", key.c_str());
		methodid = env->GetMethodID(cls, "<init>", constructorSig);
		if (methodid != 0) {
			InsertMethodID(key, methodid);
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetConstructor(%s) : failed", key.c_str());
		}
	}
}

jclass JniClassMember::GetClass(JNIEnv* env, const char* classSig) {

	jclass retCls = 0;
	Classes::iterator iter = classes_.find(classSig);
	if (iter == classes_.end()) {
		//__android_log_print(ANDROID_LOG_INFO, "JniClassMember", "GetClass(%s) : first find class", classSig);

		jclass cls = env->FindClass(classSig);
		if (cls != 0) {
			/*
			 * 在android-18中，FindClass返回的jclass不能直接缓存
			 */
			retCls = (jclass)env->NewGlobalRef(cls);
			classes_.insert(Classes::value_type(classSig, retCls));
		} else {
			__android_log_print(ANDROID_LOG_ERROR, "JniClassMember", "GetClass(%s) : failed", classSig);
		}
	} else {
		retCls = iter->second;
	}

	return retCls;
}

jfieldID JniClassMember::QueryFieldID(const std::string& key) {
	jfieldID fieldid = 0;
	Fields::iterator iter = fields_.find(key);
	if (iter != fields_.end()) {
		fieldid = iter->second;
	}

	return fieldid;
}

void JniClassMember::InsertFieldID(const std::string& key, jfieldID fieldid) {
	mu.Acquire();
	fields_.insert(Fields::value_type(key, fieldid));
	mu.Release();
}

jmethodID JniClassMember::QueryMethodID(const std::string& key) {
	jmethodID methodid = 0;
	Methods::iterator iter = methods_.find(key);
	if (iter != methods_.end()) {
		methodid = iter->second;
	}
	return methodid;
}

void JniClassMember::InsertMethodID(const std::string& key, jmethodID methodid) {
	mu.Acquire();
	methods_.insert(Methods::value_type(key, methodid));
	mu.Release();
}

std::string JniClassMember::MakeKey(const char* classsig, const char* name, const char* sig){
	std::string key(classsig);
	return key.append(" - ").append(name).append(" - ").append(sig);
}
