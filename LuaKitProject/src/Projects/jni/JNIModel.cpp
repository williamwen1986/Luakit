#include "JNIModel.h"
#include <android/log.h>
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_util.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "JniEnvWrapper.h"

namespace JNIModel
{
	namespace Boolean{
		jobject ConvertToJava(JNIEnv* env, bool source) {
			JniEnvWrapper envw(env);
			if (source) {
                jclass clazz = env->FindClass(classSig);
                jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
                jobject obj = env->NewObject(clazz, methodid, JNI_TRUE);
                return obj;
			} else {
                jclass clazz = env->FindClass(classSig);
                jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
                jobject obj = env->NewObject(clazz, methodid, JNI_FALSE);
                return obj;
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, bool& target) {
			if (env->IsSameObject(source, NULL)) {
				target = false;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallBooleanMethod(source, classSig, booleanValue.name, booleanValue.sig);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(bool)
	};

	namespace Byte {
		jobject ConvertToJava(JNIEnv* env, unsigned char source) {
			static jobject ZERO = NULL;
			if (source == 0) {
				if (ZERO == NULL) {
					jclass clazz = env->FindClass(classSig);
					jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
					jobject obj = env->NewObject(clazz, methodid, 0);
					ZERO = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				return env->NewLocalRef(ZERO);
			} else {
				jclass clazz = env->FindClass(classSig);
				jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
				return env->NewObject(clazz, methodid, source);
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, unsigned char& target) {
			if (env->IsSameObject(source, NULL)) {
				target = 0;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallByteMethod(source, classSig, byteValue.name, byteValue.sig);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(unsigned char)
	};

	namespace Integer {
		jobject ConvertToJava(JNIEnv* env, int source) {
			static jobject ZERO = NULL;
			if (source == 0) {
				if (ZERO == NULL) {
					jclass clazz = env->FindClass(classSig);
					jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
					jobject obj = env->NewObject(clazz, methodid, 0);
					ZERO = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				return env->NewLocalRef(ZERO);
			} else {
				jclass clazz = env->FindClass(classSig);
				jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
				return env->NewObject(clazz, methodid, source);
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, int& target) {
			if (env->IsSameObject(source, NULL)) {
				target = 0;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallIntMethod(source, classSig, intValue.name, intValue.sig);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(int)
	};

	namespace Float {
		jobject ConvertToJava(JNIEnv* env, float source) {
			static jobject ZERO = NULL;
			if (source == 0) {
				if (ZERO == NULL) {
					jclass clazz = env->FindClass(classSig);
					jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
					jobject obj = env->NewObject(clazz, methodid, 0);
					ZERO = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				return env->NewLocalRef(ZERO);
			} else {
				jclass clazz = env->FindClass(classSig);
				jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
				return env->NewObject(clazz, methodid, source);
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, float& target) {
			if (env->IsSameObject(source, NULL)) {
				target = 0;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallIntMethod(source, classSig, floatValue.name, floatValue.sig);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(float)
	}

	namespace Double {
		jobject ConvertToJava(JNIEnv* env, double source) {
			static jobject ZERO = NULL;
			if (source == 0) {
				if (ZERO == NULL) {
					jclass clazz = env->FindClass(classSig);
					jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
					jobject obj = env->NewObject(clazz, methodid, 0);
					ZERO = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				return env->NewLocalRef(ZERO);
			} else {
				jclass clazz = env->FindClass(classSig);
				jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
				return env->NewObject(clazz, methodid, source);
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, double& target) {
			if (env->IsSameObject(source, NULL)) {
				target = 0;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallIntMethod(source, classSig, doubleValue.name, doubleValue.sig);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(double)
	}

	namespace Long {
		jobject ConvertToJava(JNIEnv* env, long long source) {
            LOG(INFO) << "Long convertToJava, " << source;
			static jobject ZERO = NULL;
			if (source == 0) {
				if (ZERO == NULL) {
					jclass clazz = env->FindClass(classSig);
					jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
					jobject obj = env->NewObject(clazz, methodid, 0);
					ZERO = env->NewGlobalRef(obj);
					env->DeleteLocalRef(obj);
				}
				return env->NewLocalRef(ZERO);
			} else {
				jclass clazz = env->FindClass(classSig);
				jmethodID methodid = env->GetMethodID(clazz, init.name, init.sig);
				return env->NewObject(clazz, methodid, source);
			}
		}
		void ConvertToNative(JNIEnv* env, jobject source, long long& target) {
			if (env->IsSameObject(source, NULL)) {
				target = 0;
			} else {
				JniEnvWrapper envw(env);
				target = envw.CallLongMethod(source, classSig, longValue.name, longValue.sig);
			}
            
            LOG(INFO) << "Long ConvertToNative, " << target;
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(long long)
	};

	namespace String {
		jobject ConvertToJava(JNIEnv*env, const std::string& source) {
			static jstring EMPTY_STRING = NULL;
      		jstring ret = NULL;
			if (source.length() > 0) {
				if (IsStringUTF8(source)) {
					ret = env->NewStringUTF(source.c_str());
				} else {
                    LOG(ERROR) << "invalid modified UTF8 String. hex = " << base::HexEncode(source.c_str(), source.size());
                    JniEnvWrapper envw(env);
                    jbyteArray jbytes = envw->NewByteArray(source.size());
                    envw->SetByteArrayRegion(jbytes, 0, source.size(), (const jbyte*)source.data());  
                    ret = (jstring)envw.NewObject(classSig, "([BII)V", jbytes, 0, source.size());
                    envw->DeleteLocalRef(jbytes);
				}
			}
		    if (ret != NULL) {
		      return ret;
		    }
			if (EMPTY_STRING == NULL) {
				jstring str = env->NewStringUTF("");
				EMPTY_STRING = (jstring)env->NewGlobalRef(str);
				env->DeleteLocalRef(str);
			}
			return env->NewLocalRef(EMPTY_STRING);
		}
		void ConvertToNative(JNIEnv* env, jobject source, std::string& target) {
			if (env->IsSameObject(source, NULL) || env->GetStringLength((jstring)source) == 0) {
				target.assign("");
			} else {
				const char* chars = env->GetStringUTFChars((jstring)source, NULL);
				target.assign(chars);
				env->ReleaseStringUTFChars((jstring)source, chars);
			}
		}
		IMPLEMENT_DIRECT_CONVERT_TO_NATIVE(std::string)
	};

	namespace HashMap {
		template<typename KeyType, typename ValueType>
		jobject ConvertToJava(JNIEnv* env, const std::map<KeyType, ValueType>& source) {
			// To provide recursive conversion, implements in JNIConversion instead.
			// Please do not call this method.
			asm("int3");
		}
		template<typename KeyType, typename ValueType>
		void ConvertToNative(JNIEnv* env, jobject source, std::map<KeyType, ValueType>& target) {
			// To provide recursive conversion, implements in JNIConversion instead.
			// Please do not call this method.
			asm("int3");
		}
	}

	namespace SimpleEntry {
		template<typename KeyType, typename ValueType>
		jobject ConvertToJava(JNIEnv* env, const std::pair<KeyType, ValueType>& source) {
			// To provide recursive conversion, implements in JNIConversion instead.
			// Please do not call this method.
			asm("int3");
		}
		template<typename KeyType, typename ValueType>
		void ConvertToNative(JNIEnv* env, jobject source, std::pair<KeyType, ValueType>& target) {
			// To provide recursive conversion, implements in JNIConversion instead.
			// Please do not call this method.
			asm("int3");
		}
	}
};
