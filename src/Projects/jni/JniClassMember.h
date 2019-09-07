/*
 * JniClassMember.h
 *
 *  Created on: 2013-10-15
 *      Author: huangxiaogang
 */

#ifndef JNIWRAPPER_H_
#define JNIWRAPPER_H_

#include <jni.h>
#include <map>
//#include <mutex>
#include <string>

#include "base/android/jni_android.h"
#include "base/basictypes.h"
#include "base/synchronization/lock.h"
#include "base/basictypes.h"
#include "base/synchronization/lock.h"

/*
 * 	Methods保存普通函数、静态函数及构造函数。Fields保存普通成员变量及静态成员变量。
 *	Methods 和 Fields的key由classsig、name和sig组合而成，中间用'-'连接。key中包含classsig是为了区别不同类中名字和参数相同的函数或字段。
 *	最初的设想是用jclass、name和sig作为key，但对同一个obj，每次调用GetObjClass返回的jclass都不一致，因此改用classsig。另，对同一个classsig,
 *	每次调用FindClass返回的jclass是一致的。
 */
typedef std::map<std::string /*classsig - function name - sig*/, jmethodID> Methods;
typedef std::map<std::string /*classsig - field name - sig*/, jfieldID> Fields;
typedef std::map<std::string /*class sig*/, jclass> Classes;

class JniClassMember {
public:
	static JniClassMember* GetInstance();
	~JniClassMember();

	jfieldID GetStaticField(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig);
	jfieldID GetStaticField(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig);
	jfieldID GetField(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig);
	jmethodID GetStaticMethod(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig);
	jmethodID GetStaticMethod(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig);
	jmethodID GetMethod(JNIEnv* env, jobject obj, const char* classsig, const char* name, const char* sig);
	jmethodID GetMethod(JNIEnv* env, jclass cls, const char* classsig, const char* name, const char* sig);
	void GetConstructor(JNIEnv* env, const char* classSig, const char* constructorSig, jclass& cls, jmethodID& methodid);
	jclass GetClass(JNIEnv* env, const char* classSig);
private:
	static JniClassMember * m_pInstance;
	JniClassMember();

	jfieldID QueryFieldID(const std::string& key);
	void InsertFieldID(const std::string& key, jfieldID fieldid);
	jmethodID QueryMethodID(const std::string& key);
	void InsertMethodID(const std::string& key, jmethodID methodid);
	inline std::string MakeKey(const char* classsig, const char* name, const char* sig);

	Methods methods_;
	Fields fields_;
	Classes classes_;

	base::Lock mu;

	DISALLOW_COPY_AND_ASSIGN(JniClassMember);
};

#endif /* JNIWRAPPER_H_ */
