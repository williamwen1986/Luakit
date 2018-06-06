#include "JNIModel.h"
#include "JNIConversionImpl.h"

// This file contains overloaded conversion implementation.

using namespace JNIModel;

namespace JNIConversion {

	// ****** Conversion Forwarding ******

	IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(bool, Boolean)
	IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(int, Integer)
	IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(long long, Long)
	IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(std::string, String)
	IMPLEMENT_JAVAOBJECT_CONVERSION_DELEGATE(std::vector<bool>, ConvertToNativeBooleanArray, ConvertToJavaBooleanArray)
	IMPLEMENT_JAVAOBJECT_CONVERSION_DELEGATE(std::vector<unsigned char>, ConvertToNativeByteArray, ConvertToJavaByteArray)
	IMPLEMENT_JAVAOBJECT_CONVERSION_DELEGATE(std::vector<int>, ConvertToNativeIntArray, ConvertToJavaIntArray)

	// ****** Java to Native ******

	void ConvertToNative(JNIEnv* env, jboolean source, bool& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jbyte source, signed char& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jchar source, unsigned short& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jint source, int& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jlong source, long long& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jfloat source, float& target) { target = source; }
	void ConvertToNative(JNIEnv* env, jdouble source, double& target) { target = source; }


	void ConvertToNativeByteArray(JNIEnv* env, jbyteArray array, std::string& outputContainer) {
		outputContainer.clear();
		if (env->IsSameObject(array, NULL)) return;
		int length = env->GetArrayLength((jbyteArray)array);
		jbyte *element = env->GetByteArrayElements((jbyteArray)array, JNI_FALSE);
		outputContainer.append((const char*)element, length);
		env->ReleaseByteArrayElements((jbyteArray)array, element, 0);
	}

	void ConvertToNative(JNIEnv* env, jbyteArray array, std::string& outputContainer) {
		ConvertToNativeByteArray(env, array, outputContainer);
	}

	void ConvertToNativeGBKByteArray(JNIEnv* env, jstring string, std::string& outputContainer) {
		if (env->IsSameObject(string, NULL) || env->GetStringLength(string) == 0) {
			outputContainer.assign("");
		} else {
			JniEnvWrapper envw;
			jstring gbk = env->NewStringUTF("GBK");
			jbyteArray bytes = (jbyteArray)envw.CallObjectMethod(string, String::classSig, String::getBytes.name, String::getBytes.sig, gbk);
			env->DeleteLocalRef(gbk);
			ConvertToNativeByteArray(env, bytes, outputContainer);
			env->DeleteLocalRef(bytes);
		}
	}

	std::string ConvertToNativeGBKByteArray(JNIEnv* env, jstring string) {
		std::string target;
		ConvertToNativeGBKByteArray(env, string, target);
		return target;
	}

	// ****** Native to Java ******

	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const std::string& source) {
		int length = source.size();
		int index = 0;
		jbyteArray result = env->NewByteArray(length);
		jbyte *element = env->GetByteArrayElements(result, JNI_FALSE);
		memcpy(element, source.c_str(), length);
		env->ReleaseByteArrayElements(result, element, 0);
		return result;
	}

	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const void* source, int length) {
		int index = 0;
		jbyteArray result = env->NewByteArray(length);
		jbyte *element = env->GetByteArrayElements(result, JNI_FALSE);
		memcpy(element, source, length);
		env->ReleaseByteArrayElements(result, element, 0);
		return result;
	}
	
	void SetAtomicIntegerValue(JNIEnv *env, jobject integerRef, jint intValue)
	{
    	if (integerRef == NULL)
        	return;
    	jclass cls = env->GetObjectClass(integerRef);
    	jmethodID mid = env->GetMethodID(cls, "set", "(I)V");
    	if (mid == NULL){
        	env->DeleteLocalRef(cls);
        	return;
    	}
    	env->CallVoidMethod(integerRef, mid, intValue);
    	env->DeleteLocalRef(cls);
	}

	void SetAtomicLongValue(JNIEnv *env, jobject integerRef, jlong longValue)
	{
    	if (integerRef == NULL)
        	return;
    	jclass cls = env->GetObjectClass(integerRef);
    	jmethodID mid = env->GetMethodID(cls, "set", "(J)V");
    	if (mid == NULL){
        	env->DeleteLocalRef(cls);
        	return;
    	}
    	env->CallVoidMethod(integerRef, mid, longValue);
    	env->DeleteLocalRef(cls);
	}
}
