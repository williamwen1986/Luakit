#ifndef JNICONVERSIONIMPL_H
#define JNICONVERSIONIMPL_H
#include "JNIModel.h"
#include "JNIConversionDecl.h"
#include "base/logging.h"

// This file contains template conversion implementation.
#define JNICONVERSION_IMPLEMENTATION_START

using namespace JNIModel;

namespace JNIConversion {

	// ****** Java to Native ******

	template<typename TargetType>
	TargetType ConvertToNative(JNIEnv* env, jobject source) {
		TargetType target;
		ConvertToNative(env, source, target);
		return target;
	}

	template<typename TargetType>
	void ConvertToNativeEnum(JNIEnv* env, jobject source, TargetType& target) {
		JniEnvWrapper envw(env);
		if (env->IsSameObject(source, NULL)) {
			target = (TargetType)0;
		} else {
			target = (TargetType)envw.CallIntMethod(source, JavaObjectTraits<TargetType>().ClassSig(), "getNumber", "()I");
		}
	}

	template<typename ContainerType>
	void ConvertToNativeBooleanArray(JNIEnv* env, jobject array, ContainerType& outputContainer) {
		outputContainer.clear();
		if (env->IsSameObject(array, NULL)) return;
		int length = env->GetArrayLength((jbooleanArray)array);
		jboolean *element = env->GetBooleanArrayElements((jbooleanArray)array, JNI_FALSE);
		for (int i = 0; i < length; i++) {
			outputContainer.push_back(element[i]);
		}
		env->ReleaseBooleanArrayElements((jbooleanArray)array, element, 0);
	}

	template<typename ContainerType>
	void ConvertToNativeByteArray(JNIEnv* env, jobject array, ContainerType& outputContainer) {
		outputContainer.clear();
		if (env->IsSameObject(array, NULL)) return;
		int length = env->GetArrayLength((jbyteArray)array);
		jbyte *element = env->GetByteArrayElements((jbyteArray)array, JNI_FALSE);
		for (int i = 0; i < length; i++) {
			outputContainer.push_back(element[i]);
		}
		env->ReleaseByteArrayElements((jbyteArray)array, element, 0);
	}

	template<typename ContainerType>
	void ConvertToNativeIntArray(JNIEnv* env, jobject array, ContainerType& outputContainer) {
		outputContainer.clear();
		if (env->IsSameObject(array, NULL)) return;
		int length = env->GetArrayLength((jintArray)array);
		jint *element = env->GetIntArrayElements((jintArray)array, JNI_FALSE);
		for (int i = 0; i < length; i++) {
			outputContainer.push_back(element[i]);
		}
		env->ReleaseIntArrayElements((jintArray)array, element, 0);
	}
    
    template<typename ContainerType>
    void ConvertToNativeLongArray(JNIEnv* env, jlongArray array, ContainerType& outputContainer) {        
        if (env->IsSameObject(array, NULL)) {
            return;
        }
        
        int len=env->GetArrayLength(array);
        if (len > 0) {
            outputContainer.reserve(len);
            jlong *elems = env->GetLongArrayElements(array, NULL);
            for(int i = 0; i < len; i++) {
                outputContainer.push_back(elems[i]);
            }
            env->ReleaseLongArrayElements(array, elems, 0);  
        }
    }
    
	template<typename InnerType>
	void ConvertToNative(JNIEnv* env, jobject array, std::vector<InnerType>& outputContainer) {
		ConvertToNativeObjectArray(env, array, outputContainer);
	}

	template<typename InnerType>
	void ConvertToNative(JNIEnv* env, jobject array, std::list<InnerType>& outputContainer) {
		ConvertToNativeObjectArray(env, array, outputContainer);
	}
    
    template<typename InnerType>
    void ConvertToNative(JNIEnv* env, jobject array, std::set<InnerType>& outputContainer) {
        ConvertToNativeObjectSet(env, array, outputContainer);
    }

    
    template<typename ContainerType>
    void ConvertToNativeObjectSet(JNIEnv* env, jobject array, ContainerType& outputContainer){
        outputContainer.clear();
        if (env->IsSameObject(array, NULL)) return;
        int length = env->GetArrayLength((jobjectArray)array);
        for (int i = 0; i < length; i++) {

            jobject element = (jstring)env->GetObjectArrayElement((jobjectArray)array, i);
            outputContainer.insert(ConvertToNative<typename ContainerType::value_type>(env, element));
            env->DeleteLocalRef(element);
        }
    }
    
	template<typename ContainerType>
	void ConvertToNativeObjectArray(JNIEnv* env, jobject array, ContainerType& outputContainer) {
		outputContainer.clear();
		if (env->IsSameObject(array, NULL)) return;
		int length = env->GetArrayLength((jobjectArray)array);
		for (int i = 0; i < length; i++) {

			jobject element = (jstring)env->GetObjectArrayElement((jobjectArray)array, i);
			outputContainer.push_back(ConvertToNative<typename ContainerType::value_type>(env, element));
			env->DeleteLocalRef(element);
		}
	}

	template<typename FirstType, typename SecondType>
	void ConvertToNativeEntry(JNIEnv* env, jobject source, std::pair<FirstType, SecondType>& target) {
		if (env->IsSameObject(source, NULL)) {
			return;
		} else {
			JniEnvWrapper envw(env);

			jobject first = envw.CallObjectMethod(source, Entry::classSig, Entry::getKey.name, Entry::getKey.sig);
			jobject second = envw.CallObjectMethod(source, Entry::classSig, Entry::getValue.name, Entry::getValue.sig);
			ConvertToNative(env, first, target.first);
			ConvertToNative(env, second, target.second);
			env->DeleteLocalRef(first);
			env->DeleteLocalRef(second);
		}
	}

	template<typename FirstType, typename SecondType>
	void ConvertToNative(JNIEnv* env, jobject source, std::pair<FirstType, SecondType>& target) {
		ConvertToNativeEntry(env, source, target);
	}

	template<typename KeyType, typename ValueType, typename Compare>
	void ConvertToNativeMap(JNIEnv* env, jobject source, std::map<KeyType, ValueType, Compare>& target) {
		target.clear();
		if (!env->IsSameObject(source, NULL)) {
			JniEnvWrapper envw(env);
			env->PushLocalFrame(0);
			jobject entryset = envw.CallObjectMethod(source, HashMap::classSig, HashMap::entrySet.name, HashMap::entrySet.sig);
			jobject iterator = envw.CallObjectMethod(entryset, Set::classSig, Set::iterator.name, Set::iterator.sig);
			JniClassMember* jcm = JniClassMember::GetInstance();
			while (true) {

				jboolean hasNext = envw.CallBooleanMethod(iterator, Iterator::classSig, Iterator::hasNext.name, Iterator::hasNext.sig);
				if (hasNext == JNI_FALSE) {

					break;
				}
				jobject entry = envw.CallObjectMethod(iterator, Iterator::classSig, Iterator::next.name, Iterator::next.sig);
				jobject key = env->CallObjectMethod(entry, jcm->GetMethod(env, jcm->GetClass(env, Entry::classSig), Entry::classSig, Entry::getKey.name, Entry::getKey.sig));
				jobject val = env->CallObjectMethod(entry, jcm->GetMethod(env, jcm->GetClass(env, Entry::classSig), Entry::classSig, Entry::getValue.name, Entry::getValue.sig));
				target.insert(std::make_pair(ConvertToNative<KeyType>(env, key), ConvertToNative<ValueType>(env, val)));
				env->DeleteLocalRef(entry);
				env->DeleteLocalRef(key);
				env->DeleteLocalRef(val);
			}
			env->PopLocalFrame(NULL);
		}
	}

	template<typename KeyType, typename ValueType, typename Compare>
	void ConvertToNative(JNIEnv* env, jobject source, std::map<KeyType, ValueType, Compare>& target) {
		ConvertToNativeMap(env, source, target);
	}

	// ****** Native to Java ******

	template<typename SourceType>
	jobject ConvertToJavaEnum(JNIEnv* env, const SourceType& source) {
		JniEnvWrapper envw(env);
		return envw.CallStaticObjectMethod(JavaObjectTraits<SourceType>().ClassSig(), "valueOf", std::string("(I)").append(JavaObjectTraits<SourceType>().FieldSig()).c_str(), (int)source);
	}

	template<typename ContainerType>
	jbooleanArray ConvertToJavaBooleanArray(JNIEnv* env, const ContainerType& source) {
		int length = source.size();
		int index = 0;
		jbooleanArray result = env->NewBooleanArray(length);
		jboolean *element = env->GetBooleanArrayElements(result, JNI_FALSE);
		for (typename ContainerType::const_iterator iter = source.begin(); iter != source.end(); ++iter) {
			element[index] = *iter;
			++index;
		}
		env->ReleaseBooleanArrayElements(result, element, 0);
		return result;
	}

	template<typename ContainerType>
	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const ContainerType& source) {
		int length = source.size();
		int index = 0;
		jbyteArray result = env->NewByteArray(length);
		jbyte *element = env->GetByteArrayElements(result, JNI_FALSE);
		for (typename ContainerType::const_iterator iter = source.begin(); iter != source.end(); ++iter) {
			element[index] = *iter;
			++index;
		}
		env->ReleaseByteArrayElements(result, element, 0);
		return result;
	}

	template <typename ProtobufType>
	jbyteArray ConvertProtobufToJavaByteArray(JNIEnv* env, const ProtobufType* source) {
        int byteSize = source->ByteSize();
        if (byteSize > 0) {
            jbyte* output = new jbyte[byteSize];
            source->SerializeToArray(output, byteSize);

            jbyteArray result = env->NewByteArray(byteSize);
            env->SetByteArrayRegion(result, 0, byteSize, output);
            delete []output;
            return result;
        } else {
            return nullptr;
        }
    }

	template<typename ContainerType>
	jintArray ConvertToJavaIntArray(JNIEnv* env, const ContainerType& source) {
		int length = source.size();
		int index = 0;
		jintArray result = env->NewIntArray(length);
		jint *element = env->GetIntArrayElements(result, JNI_FALSE);
		for (typename ContainerType::const_iterator iter = source.begin(); iter != source.end(); ++iter) {
			element[index] = *iter;
			++index;
		}
		env->ReleaseIntArrayElements(result, element, 0);
		return result;
	}
    
	template<typename ContainerType>
	jlongArray ConvertToJavaLongArray(JNIEnv* env, const ContainerType& source) {
		int length = source.size();
		int index = 0;
		jlongArray result = env->NewLongArray(length);
		jlong *element = env->GetLongArrayElements(result, JNI_FALSE);
		for (typename ContainerType::const_iterator iter = source.begin(); iter != source.end(); ++iter) {
			element[index] = *iter;
			++index;
		}
		env->ReleaseLongArrayElements(result, element, 0);
		return result;
	}
    
	template<typename ContainerType>
	jobjectArray ConvertToJavaObjectArray(JNIEnv* env, const ContainerType& source, const char* classsig) {
		JniEnvWrapper envw(env);
		int length = source.size();
		if (length == 0) {
			return NULL;
		}
		
		int index = 0;
		jobjectArray result = envw.NewObjectArray(classsig, length, NULL);
		for (typename ContainerType::const_iterator iter = source.begin(); iter != source.end(); ++iter) {
			//env->PushLocalFrame(0);
			jobject element = ConvertToJava(env, (const typename ContainerType::value_type&)*iter);
			env->SetObjectArrayElement(result, index, element);
			env->DeleteLocalRef(element);
			++index;
		}
		return result;
	}

	template<typename InnerType>
	jobjectArray ConvertToJava(JNIEnv* env, const std::vector<InnerType>& source, const char* classsig) {
		return ConvertToJavaObjectArray(env, source, classsig);
	}

	template<typename InnerType>
	jobjectArray ConvertToJava(JNIEnv* env, const std::list<InnerType>& source, const char* classsig) {
		return ConvertToJavaObjectArray(env, source, classsig);
	}
    
    template<typename InnerType>
    jobjectArray ConvertToJava(JNIEnv* env, const std::set<InnerType>& source, const char* classsig) {
        return ConvertToJavaObjectArray(env, source, classsig);
    }

	template<typename FirstType, typename SecondType>
	jobject ConvertToJavaEntry(JNIEnv* env, const std::pair<FirstType, SecondType>& source) {
		JniEnvWrapper envw(env);

		jobject key = ConvertToJava(env, source.first);
		jobject val = ConvertToJava(env, source.second);
		jobject ret = envw.NewObject(SimpleEntry::classSig, SimpleEntry::init.sig, key, val);

		env->DeleteLocalRef(key);
		env->DeleteLocalRef(val);

		return ret;
	}

	template<typename FirstType, typename SecondType>
	jobject ConvertToJava(JNIEnv* env, const std::pair<FirstType, SecondType>& source) {
		return ConvertToJavaEntry(env, source);
	}

	template<typename KeyType, typename ValueType>
	jobject ConvertToJavaMap(JNIEnv* env, const std::map<KeyType, ValueType>& source) {
		JniEnvWrapper envw(env);
		jclass clazz = env->FindClass(HashMap::classSig);
		jmethodID methodid = env->GetMethodID(clazz, HashMap::init.name, HashMap::init.sig);
		jobject map = env->NewObject(clazz, methodid);
		for (typename std::map<KeyType, ValueType>::const_iterator iter = source.begin(); iter != source.end(); ++iter) {

			jobject key = ConvertToJava(env, iter->first);
			jobject val = ConvertToJava(env, iter->second);
			jobject ret = envw.CallObjectMethod(map, HashMap::classSig, HashMap::put.name, HashMap::put.sig, key, val);

			env->DeleteLocalRef(key);
			env->DeleteLocalRef(val);
			env->DeleteLocalRef(ret);
		}
		return map;
	}

	template<typename KeyType, typename ValueType>
	jobject ConvertToJava(JNIEnv* env, const std::map<KeyType, ValueType>& source) {
		ConvertToJavaMap(env, source);
	}

}
#endif
