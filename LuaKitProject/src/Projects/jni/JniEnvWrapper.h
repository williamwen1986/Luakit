/*
 * JniEnvWrapper.h
 *
 *  Created on: 2013-10-15
 *      Author: huangxiaogang
 */

#ifndef JNIENVWRAPPER_H_
#define JNIENVWRAPPER_H_
#include <jni.h>


class JniEnvWrapper {
public:
	// This constructor store the giving JavaVM, and use the JavaVM object to get a JNIEnv instance.
	// If the current thread has not been attached with JavaVM, do attaching and get a JNIEnv instance. Then detach on destruction.
	JniEnvWrapper(JavaVM* jvm);
	// This constructor use the giving JNIEnv instance.
	JniEnvWrapper(JNIEnv* env);
	// This constructor use previous stored JavaVM to get JNIEnv instance.
	// If the current thread has not been attached with JavaVM, do attaching and get a JNIEnv instance. Then detach on destruction.
	JniEnvWrapper();
	JniEnvWrapper(JniEnvWrapper& another);
	virtual ~JniEnvWrapper();

	JniEnvWrapper& operator=(const JniEnvWrapper &another);
	operator JNIEnv*() const;
	JNIEnv* operator*() const;
	JNIEnv* operator->() const;
	static JavaVM* GetJavaVM();
	static void SetJavaVM(JavaVM* jvm);

	// get field
	jint 		GetIntField(jobject obj, const char* classsig, const char* name);
	jboolean 	GetBoolField(jobject obj, const char* classsig, const char* name);
	jbyte 	 	GetByteField(jobject obj, const char* classsig, const char* name);
	jchar 	 	GetCharField(jobject obj, const char* classsig, const char* name);
	jshort 	 	GetShortField(jobject obj, const char* classsig, const char* name);
	jlong 		GetLongField(jobject obj, const char* classsig, const char* name);
	jfloat 		GetFloatField(jobject obj, const char* classsig, const char* name);
	jdouble 	GetDoubleField(jobject obj, const char* classsig, const char* name);
	jobject 	GetObjectField(jobject obj, const char* classsig, const char* name, const char* sig);
	jstring 	GetStringField(jobject obj, const char* classsig, const char* name);

	// get static field
	jint 		GetStaticIntField(jobject obj, const char* classsig, const char* name);
	jboolean 	GetStaticBoolField(jobject obj, const char* classsig, const char* name);
	jbyte 		GetStaticByteField(jobject obj, const char* classsig, const char* name);
	jchar 		GetStaticCharField(jobject obj, const char* classsig, const char* name);
	jshort 		GetStaticShortField(jobject obj, const char* classsig, const char* name);
	jlong 		GetStaticLongField(jobject obj, const char* classsig, const char* name);
	jfloat 		GetStaticFloatField(jobject obj, const char* classsig, const char* name);
	jdouble 	GetStaticDoubleField(jobject obj, const char* classsig, const char* name);
	jobject 	GetStaticObjectField(jobject obj, const char* classsig, const char* name, const char* sig);
	jstring 	GetStaticStringField(jobject obj, const char* classsig, const char* name);

	// set field
	void SetIntField(jobject obj, const char* classsig, const char* name, jint value);
	void SetBoolField(jobject obj, const char* classsig, const char* name, jboolean value);
	void SetByteField(jobject obj, const char* classsig, const char* name, jbyte value);
	void SetCharField(jobject obj, const char* classsig, const char* name, jchar value);
	void SetShortField(jobject obj, const char* classsig, const char* name, jshort value);
	void SetLongField(jobject obj, const char* classsig, const char* name, jlong value);
	void SetFloatField(jobject obj, const char* classsig, const char* name, jfloat value);
	void SetDoubleField(jobject obj, const char* classsig, const char* name, jdouble value);
	void SetObjectField(jobject obj, const char* classsig, const char* name, const char* sig, jobject value);
	void SetStringField(jobject obj, const char* classsig, const char* name, jstring value);

	// set static field
	void SetStaticIntField(jobject obj, const char* classsig, const char* name, jint value);
	void SetStaticBoolField(jobject obj, const char* classsig, const char* name, jboolean value);
	void SetStaticByteField(jobject obj, const char* classsig, const char* name, jbyte value);
	void SetStaticCharField(jobject obj, const char* classsig, const char* name, jchar value);
	void SetStaticShortField(jobject obj, const char* classsig, const char* name, jshort value);
	void SetStaticLongField(jobject obj, const char* classsig, const char* name, jlong value);
	void SetStaticFloatField(jobject obj, const char* classsig, const char* name, jfloat value);
	void SetStaticDoubleField(jobject obj, const char* classsig, const char* name, jdouble value);
	void SetStaticObjectField(jobject obj, const char* classsig, const char* name, const char* sig, jobject value);
	void SetStaticStringField(jobject obj, const char* classsig, const char* name, jstring value);

	// get array field
	jobjectArray 	GetObjectArray(jobject obj, const char* classsig, const char* name, const char* sig);
	jobjectArray	GetStringArray(jobject obj, const char* classsig, const char* name);
	jbooleanArray 	GetBoolArray(jobject obj, const char* classsig, const char* name);
	jbyteArray 		GetByteArray(jobject obj, const char* classsig, const char* name);
	jcharArray 		GetCharArray(jobject obj, const char* classsig, const char* name);
	jshortArray 	GetShortArray(jobject obj, const char* classsig, const char* name);
	jintArray 		GetIntArray(jobject obj, const char* classsig, const char* name);
	jlongArray 		GetLongArray(jobject obj, const char* classsig, const char* name);
	jfloatArray 	GetFloatArray(jobject obj, const char* classsig, const char* name);
	jdoubleArray 	GetDoubleArray(jobject obj, const char* classsig, const char* name);

	void 			SetObjectArray(jobject obj, const char* classsig, const char* name, const char* sig, jobjectArray value);
	void 			SetStringArray(jobject obj, const char* classsig, const char* name, jobjectArray value);
	void 			SetBoolArray(jobject obj, const char* classsig, const char* name, jbooleanArray value);
	void 			SetByteArray(jobject obj, const char* classsig, const char* name, jbyteArray value);
	void 			SetByteArrayArray(jobject obj, const char* classsig, const char* name, jobjectArray value);
	void 			SetCharArray(jobject obj, const char* classsig, const char* name, jcharArray value);
	void 			SetShortArray(jobject obj, const char* classsig, const char* name, jshortArray value);
	void 			SetIntArray(jobject obj, const char* classsig, const char* name, jintArray value);
	void 			SetLongArray(jobject obj, const char* classsig, const char* name, jlongArray value);
	void 			SetFloatArray(jobject obj, const char* classsig, const char* name, jfloatArray value);
	void 			SetDoubleArray(jobject obj, const char* classsig, const char* name, jdoubleArray value);

	// call method
    jobject     CallObjectMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jboolean    CallBooleanMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jbyte       CallByteMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jchar       CallCharMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jshort      CallShortMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jint        CallIntMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jlong       CallLongMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jfloat      CallFloatMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jdouble     CallDoubleMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    void        CallVoidMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);

    // call static method
    jobject 	CallStaticObjectMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jboolean 	CallStaticBooleanMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jbyte 		CallStaticByteMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jchar 		CallStaticCharMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jshort 		CallStaticShortMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jint 		CallStaticIntMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jlong 		CallStaticLongMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jfloat 		CallStaticFloatMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    jdouble 	CallStaticDoubleMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);
    void 		CallStaticVoidMethod(jobject obj, const char* classsig, const char* name, const char* sig, ...);

	jobject 	CallStaticObjectMethod(const char* classsig, const char* name, const char* sig, ...);
    jboolean 	CallStaticBooleanMethod(const char* classsig, const char* name, const char* sig, ...);
    jbyte 		CallStaticByteMethod(const char* classsig, const char* name, const char* sig, ...);
    jchar 		CallStaticCharMethod(const char* classsig, const char* name, const char* sig, ...);
    jshort 		CallStaticShortMethod(const char* classsig, const char* name, const char* sig, ...);
    jint 		CallStaticIntMethod(const char* classsig, const char* name, const char* sig, ...);
    jlong 		CallStaticLongMethod(const char* classsig, const char* name, const char* sig, ...);
    jfloat 		CallStaticFloatMethod(const char* classsig, const char* name, const char* sig, ...);
    jdouble 	CallStaticDoubleMethod(const char* classsig, const char* name, const char* sig, ...);
    void 		CallStaticVoidMethod(const char* classsig, const char* name, const char* sig, ...);

    // new object
    jobject NewObject(const char* classSig, const char* constructorSig, ...);
    jobjectArray NewObjectArray(const char* classSig, int elementCount, jobject initialElement);

private:
    void Init();
	JNIEnv* env_;
	bool attached_;
	static JavaVM* jvm_;
};

#endif /* JNIENVWRAPPER_H_ */
