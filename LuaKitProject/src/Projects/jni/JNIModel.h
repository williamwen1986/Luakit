#ifndef JNIMODEL_H
#define JNIMODEL_H
#include <jni.h>
#include <string>
#include <map>
#include "JniEnvWrapper.h"

#define DEFINE_SIGNATURE(classsig) \
static const char* classSig = classsig; \
static const char* fieldSig = "L" classsig ";"; \
static const char* arraySig = "[L" classsig ";";

#define DECLARE_DIRECT_CONVERT_TO_NATIVE(type) type ConvertToNative(JNIEnv* env, jobject source);

#define IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(type) \
type ConvertToNative(JNIEnv* env, jobject source) { \
	type result; \
	ConvertToNative(env, source, result); \
	return result; \
}

namespace JNIModel {

	typedef struct _function {
		const char* name;
		const char* sig;
	} Function;

	namespace Boolean {
		DEFINE_SIGNATURE("java/lang/Boolean")
		static const Function init = { "<init>", "(Z)V" };
		static const Function booleanValue = { "booleanValue", "()Z" };
		jobject ConvertToJava(JNIEnv* env, bool source);
		void ConvertToNative(JNIEnv* env, jobject source, bool& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(bool)
	}

	namespace Byte {
		DEFINE_SIGNATURE("java/lang/Byte")
		static const Function init = { "<init>", "(B)V" };
		static const Function byteValue = { "byteValue", "()B" };
		jobject ConvertToJava(JNIEnv* env, unsigned char source);
		void ConvertToNative(JNIEnv* env, jobject source, unsigned char& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(unsigned char)
	}

	namespace Integer {
		DEFINE_SIGNATURE("java/lang/Integer");
		static const Function init = { "<init>", "(I)V" };
		static const Function intValue = { "intValue", "()I" };
		jobject ConvertToJava(JNIEnv* env, int source);
		void ConvertToNative(JNIEnv* env, jobject source, int& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(int)
	}

	namespace Float {
		DEFINE_SIGNATURE("java/lang/Float");
		static const Function init = { "<init>", "(F)V" };
		static const Function floatValue = { "floatValue", "()F" };
		jobject ConvertToJava(JNIEnv* env, float source);
		void ConvertToNative(JNIEnv* env, jobject source, float& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(float)
	}

	namespace Double {
		DEFINE_SIGNATURE("java/lang/Double");
		static const Function init = { "<init>", "(D)V" };
		static const Function doubleValue = { "doubleValue", "()D" };
		jobject ConvertToJava(JNIEnv* env, double source);
		void ConvertToNative(JNIEnv* env, jobject source, double& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(double)
	}

	namespace Long {
		DEFINE_SIGNATURE("java/lang/Long");
		static const Function init = { "<init>", "(J)V" };
		static const Function longValue = { "longValue", "()J" };
		jobject ConvertToJava(JNIEnv* env, long long source);
		void ConvertToNative(JNIEnv* env, jobject source, long long& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(long long)
	}
	namespace String {
		DEFINE_SIGNATURE("java/lang/String");
		static const Function getBytes = { "getBytes", "(Ljava/lang/String;)[B" };
		jobject ConvertToJava(JNIEnv*env, const std::string& source);
		void ConvertToNative(JNIEnv* env, jobject source, std::string& target);
		DECLARE_DIRECT_CONVERT_TO_NATIVE(std::string)
	}

	namespace Set {
		DEFINE_SIGNATURE("java/util/Set");
		static const Function iterator = { "iterator", "()Ljava/util/Iterator;" };
	}

	namespace Iterator {
		DEFINE_SIGNATURE("java/util/Iterator");
		static const Function hasNext = { "hasNext", "()Z" };
		static const Function next = { "next", "()Ljava/lang/Object;" };
	}

	namespace Entry {
		DEFINE_SIGNATURE("java/util/Map$Entry");
		static const Function getKey = { "getKey", "()Ljava/lang/Object;" };
		static const Function getValue = { "getValue", "()Ljava/lang/Object;" };
	}

	namespace HashMap {
		DEFINE_SIGNATURE("java/util/HashMap");
		static const Function init = { "<init>", "()V" };
		static const Function entrySet = { "entrySet", "()Ljava/util/Set;" };
		static const Function put = { "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" };
		template<typename KeyType, typename ValueType>
		jobject ConvertToJava(JNIEnv* env, const std::map<KeyType, ValueType>& source);
		template<typename KeyType, typename ValueType>
		void ConvertToNative(JNIEnv* env, jobject source, std::map<KeyType, ValueType>& target);
	}

	namespace SimpleEntry {
		DEFINE_SIGNATURE("java/util/AbstractMap$SimpleEntry");
		static const Function init = { "<init>", "(Ljava/lang/String;Ljava/lang/String;)V" };
		template<typename KeyType, typename ValueType>
		jobject ConvertToJava(JNIEnv* env, const std::pair<KeyType, ValueType>& source);
		template<typename KeyType, typename ValueType>
		void ConvertToNative(JNIEnv* env, jobject source, std::pair<KeyType, ValueType>& target);
	}

	namespace Date {
		DEFINE_SIGNATURE("java/util/Date");
		static const Function getTime = { "getTime", "()J" };
	}
}
#endif
