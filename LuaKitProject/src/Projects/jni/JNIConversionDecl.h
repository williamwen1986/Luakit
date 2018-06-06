#ifndef JNICONVERSIONDECL_H
#define JNICONVERSIONDECL_H
#include <jni.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <functional>
#include "JNIModel.h"
#include "JniClassMember.h"

// This file contains template/overloaded conversion declaration.

#ifdef JNICONVERSION_IMPLEMENTATION_START
#error Any conversion implementation header should be placed after conversion declaration header!
#error You should include all "*Decl.h" first, then include all "*Impl.h".
#endif

//map���uid������搴�瑙����
struct UidLessCompare;

using namespace JNIModel;

#define DECLARE_JAVAOBJECT_TRAITS_TEMPLATE(valuetype, classsig, fieldsig, arraysig) \
template<> \
struct JavaObjectTraits<valuetype > { \
	inline const char* ClassSig() { return classsig; } \
	inline const char* FieldSig() { return fieldsig; } \
	inline const char* ArraySig() { return arraysig; } \
};

// Native => Java
#define DECLARE_OVERLOADED_TONATIVE_CONVERSION(valuetype) \
void ConvertToNative(JNIEnv* env, jobject source, valuetype& target);

#define IMPLEMENT_OVERLOADED_TONATIVE_CONVERSION(valuetype, delegate) \
void ConvertToNative(JNIEnv* env, jobject source, valuetype& target) { \
	delegate(env, source, target); \
}

#define IMPLEMENT_OVERLOADED_PB_TO_NATIVE_CONVERSION(valuetype, delegate, forwardtype, forwarder) \
void ConvertToNative(JNIEnv* env, jobject source, valuetype& target) { \
  forwardtype* pbtarget = new forwardtype(); \
	delegate(env, source, *pbtarget); \
  scoped_refptr<valuetype> t (&target); \
  forwarder::ConvertFromProtobufToCpp(pbtarget, t); \
  delete pbtarget; \
}

// Java => Native
#define DECLARE_OVERLOADED_TOJAVA_CONVERSION(valuetype) \
jobject ConvertToJava(JNIEnv* env, const valuetype& source);

#define IMPLEMENT_OVERLOADED_TOJAVA_CONVERSION(valuetype, delegate) \
jobject ConvertToJava(JNIEnv* env, const valuetype& source) { \
	return delegate(env, source); \
}

#define IMPLEMENT_OVERLOADED_NATIVE_TO_PB_CONVERSION(valuetype, delegate, forwardtype, forwarder) \
jobject ConvertToJava(JNIEnv* env, const valuetype& source) { \
  forwardtype* pbtarget = new forwardtype(); \
  forwarder::ConvertFromCppToProtobuf(&source, pbtarget); \
	return delegate(env, *pbtarget); \
}

// Java <=> Native {{{
// Define Java <=> Native for native type, ProtocolResult etc.
#define DECLARE_JAVAOBJECT_CONVERSION_MODEL(valuetype, modelnamespace) \
	DECLARE_JAVAOBJECT_TRAITS_TEMPLATE(valuetype, modelnamespace::classSig, modelnamespace::fieldSig, modelnamespace::arraySig) \
	DECLARE_OVERLOADED_TONATIVE_CONVERSION(valuetype) \
	DECLARE_OVERLOADED_TOJAVA_CONVERSION(valuetype) \

// Impl Java <=> Native for native type, ProtocolResult etc.
#define IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(valuetype, modelnamespace) \
	IMPLEMENT_OVERLOADED_TONATIVE_CONVERSION(valuetype, modelnamespace::ConvertToNative) \
	IMPLEMENT_OVERLOADED_TOJAVA_CONVERSION(valuetype, modelnamespace::ConvertToJava) \
// }}}

// Java <=> Native for protobuf {{{
// Define Java <=> Native for protobuf type, UMA etc.
#define DECLARE_JAVAOBJECT_CONVERSION_SIGNATURE(valuetype, classsig) \
	DECLARE_JAVAOBJECT_TRAITS_TEMPLATE(valuetype, classsig, "L" classsig ";", "[L" classsig ";") \
	DECLARE_OVERLOADED_TONATIVE_CONVERSION(valuetype) \
	DECLARE_OVERLOADED_TOJAVA_CONVERSION(valuetype) \

// Impl Java <=> Native for protobuf type, UMA etc.
#define IMPLEMENT_JAVAOBJECT_CONVERSION_DELEGATE(valuetype, tonative_delegate, tojava_delegate) \
	IMPLEMENT_OVERLOADED_TONATIVE_CONVERSION(valuetype, tonative_delegate) \
	IMPLEMENT_OVERLOADED_TOJAVA_CONVERSION(valuetype, tojava_delegate) \
// }}}

// Protobuf <=> Native {{{
// Define Protobuf <=> Native for protobuf type, UMA etc.
#define DECLARE_JAVA_PB_CONVERSION_SIGNATURE(valuetype, forwardtype, classsig) \
	DECLARE_JAVAOBJECT_TRAITS_TEMPLATE(forwardtype, classsig, "L" classsig ";", "[L" classsig ";") \
	DECLARE_OVERLOADED_TONATIVE_CONVERSION(forwardtype) \
	DECLARE_OVERLOADED_TOJAVA_CONVERSION(forwardtype) \
	DECLARE_OVERLOADED_TONATIVE_CONVERSION(valuetype) \
	DECLARE_OVERLOADED_TOJAVA_CONVERSION(valuetype) \
// Impl Protobuf <=> Native for protobuf type, for Calendar.
#define IMPLEMENT_JAVA_PB_CONVERSION_DELEGATE(valuetype, tonative_delegate, tojava_delegate, forwardtype, forwarder) \
	IMPLEMENT_OVERLOADED_PB_TO_NATIVE_CONVERSION(valuetype, tonative_delegate, forwardtype, forwarder) \
	IMPLEMENT_OVERLOADED_NATIVE_TO_PB_CONVERSION(valuetype, tojava_delegate, forwardtype, forwarder) \
	IMPLEMENT_OVERLOADED_TONATIVE_CONVERSION(forwardtype, tonative_delegate) \
	IMPLEMENT_OVERLOADED_TOJAVA_CONVERSION(forwardtype, tojava_delegate) \
// }}}

#define ENUM_TO_NATIVE(envw, classsig, obj) \
		envw.CallIntMethod(obj, classsig, "getNumber", "()I");

#define ENUM_TO_JAVA(envw, classsig, value) \
		envw.CallStaticObjectMethod((classsig), "valueOf", "(I)L" classsig ";", (value));

namespace JNIConversion {

	template<typename TargetType>
	struct ConversionIsNotSupported { };

	template<typename TargetType>
	struct JavaObjectTraits {
		inline const char* ClassSig() {
			typedef typename ConversionIsNotSupported<TargetType>::ASSERT_java_object_traits_have_not_been_defined_for_target_type compile_error;
			return NULL;
		}
		inline const char* FieldSig() {
			typedef typename ConversionIsNotSupported<TargetType>::ASSERT_java_object_traits_have_not_been_defined_for_target_type compile_error;
			return NULL;
		}
		inline const char* ArraySig() {
			typedef typename ConversionIsNotSupported<TargetType>::ASSERT_java_object_traits_have_not_been_defined_for_target_type compile_error;
			return NULL;
		}
	};

	// ****** Conversion Forwarding ******

	DECLARE_JAVAOBJECT_CONVERSION_MODEL(bool, Boolean)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(int, Integer)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(std::string, String)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(std::vector<bool>, Boolean)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(std::vector<unsigned char>, Byte)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(std::vector<int>, Integer)
	DECLARE_JAVAOBJECT_CONVERSION_MODEL(long long, Long)

	// ****** Java to Native ******

	void ConvertToNative(JNIEnv* env, jboolean source, bool& target);
	void ConvertToNative(JNIEnv* env, jbyte source, signed char& target);
	void ConvertToNative(JNIEnv* env, jchar source, unsigned short& target);
	void ConvertToNative(JNIEnv* env, jint source, int& target);
	void ConvertToNative(JNIEnv* env, jlong source, long long& target);
	void ConvertToNative(JNIEnv* env, jfloat source, float& target);
	void ConvertToNative(JNIEnv* env, jdouble source, double& target);

	void SetAtomicIntegerValue(JNIEnv *env, jobject integerRef, jint intValue);
	void SetAtomicLongValue(JNIEnv *env, jobject integerRef, jlong longValue);
	
	template<typename TargetType>
	void ConvertToNative(JNIEnv* env, jobject source, TargetType& target) {
		// ATTENTION PLEASE!
		// You are trapped here in compile-time because the source cannot be converted to TargetType automatically.
		// You can find the TargetType in compile log message as "ConversionIsNotSupported<TargetType>".
		typedef typename ConversionIsNotSupported<TargetType>::ASSERT_automatic_conversion_to_native_is_not_supported_for_target_type compile_error;
	}

	template<typename TargetType>
	TargetType ConvertToNative(JNIEnv* env, jobject source);

	template<typename TargetType>
	void ConvertToNativeEnum(JNIEnv* env, jobject source, TargetType& target);

	template<typename ContainerType>
	void ConvertToNativeBooleanArray(JNIEnv* env, jobject array, ContainerType& outputContainer);

	template<typename ContainerType>
	void ConvertToNativeByteArray(JNIEnv* env, jobject array, ContainerType& outputContainer);
	void ConvertToNativeByteArray(JNIEnv* env, jobject array, std::string& outputContainer);
	void ConvertToNativeGBKByteArray(JNIEnv* env, jstring string, std::string& outputContainer);
	std::string ConvertToNativeGBKByteArray(JNIEnv* env, jstring string);

    template<typename ContainerType>
    void ConvertToNativeLongArray(JNIEnv* env, jlongArray array, ContainerType& outputContainer);
    
	template<typename ContainerType>
	void ConvertToNativeIntArray(JNIEnv* env, jobject array, ContainerType& outputContainer);

	template<typename ContainerType>
	void ConvertToNativeObjectArray(JNIEnv* env, jobject array, ContainerType& outputContainer);
    
    template<typename ContainerType>
    void ConvertToNativeObjectSet(JNIEnv* env, jobject array, ContainerType& outputContainer);

	template<typename InnerType>
	void ConvertToNative(JNIEnv* env, jobject array, std::vector<InnerType>& outputContainer);

	template<typename InnerType>
	void ConvertToNative(JNIEnv* env, jobject array, std::list<InnerType>& outputContainer);
    
    template<typename InnerType>
    void ConvertToNative(JNIEnv* env, jobject array, std::set<InnerType>& outputContainer);

	void ConvertToNative(JNIEnv* env, jbyteArray array, std::string& outputContainer);

	template<typename FirstType, typename SecondType>
	void ConvertToNativeEntry(JNIEnv* env, jobject source, std::pair<FirstType, SecondType>& target);

	template<typename FirstType, typename SecondType>
	void ConvertToNative(JNIEnv* env, jobject source, std::pair<FirstType, SecondType>& target);

	template<typename KeyType, typename ValueType, typename Compare>
	void ConvertToNativeMap(JNIEnv* env, jobject source, std::map<KeyType, ValueType, Compare>& target);

//	template<typename KeyType, typename ValueType>
//	void ConvertToNativeMap(JNIEnv* env, jobject source, std::map<KeyType, ValueType, UidLessCompare>& target);

	template<typename KeyType, typename ValueType, typename Compare>
	void ConvertToNative(JNIEnv* env, jobject source, std::map<KeyType, ValueType, Compare>& target);

//	template<typename KeyType, typename ValueType>
//	void ConvertToNative(JNIEnv* env, jobject source, std::map<KeyType, ValueType, UidLessCompare>& target);

	template<typename TargetType>
	void NotImplemented(JNIEnv* env, TargetType& source) {
		typedef typename ConversionIsNotSupported<TargetType>::ASSERT_illegal_call_to_unimplemented_conversion_method_for_target_type compile_error;
	}

	// ****** Native to Java ******

	template<typename SourceType>
	jobject ConvertToJava(JNIEnv* env, const SourceType& source) {
		// ATTENTION PLEASE!
		// You are trapped here in compile-time because the SourceType cannot be converted to Java type automatically.
		// You can find the TargetType in compile log message as "ConversionIsNotSupported<SourceType>".
		typedef typename ConversionIsNotSupported<SourceType>::ASSERT_automatic_conversion_to_java_is_not_supported_for_source_type compile_error;
	}

	template<typename SourceType>
	jobject ConvertToJavaEnum(JNIEnv* env, const SourceType& source);

	template<typename ContainerType>
	jbooleanArray ConvertToJavaBooleanArray(JNIEnv* env, const ContainerType& source);

	template<typename ContainerType>
	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const ContainerType& source);
	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const std::string& source);
	jbyteArray ConvertToJavaByteArray(JNIEnv* env, const void* source, int length);

	template<typename ContainerType>
	jintArray ConvertToJavaIntArray(JNIEnv* env, const ContainerType& source);

	template<typename ContainerType>
	jlongArray ConvertToJavaLongArray(JNIEnv* env, const ContainerType& source);
    
	template<typename ContainerType>
	jobjectArray ConvertToJavaObjectArray(JNIEnv* env, const ContainerType& source, const char* classsig = JavaObjectTraits<typename ContainerType::value_type>().ClassSig());

	template<typename InnerType>
	jobjectArray ConvertToJava(JNIEnv* env, const std::vector<InnerType>& source, const char* classsig = JavaObjectTraits<InnerType>().ClassSig());

	template<typename InnerType>
	jobjectArray ConvertToJava(JNIEnv* env, const std::list<InnerType>& source, const char* classsig = JavaObjectTraits<InnerType>().ClassSig());
    
    template<typename InnerType>
    jobjectArray ConvertToJava(JNIEnv* env, const std::set<InnerType>& source, const char* classsig = JavaObjectTraits<InnerType>().ClassSig());

	template<typename KeyType, typename ValueType>
	jobject ConvertToJavaEntry(JNIEnv* env, const std::pair<KeyType, ValueType>& source);

	template<typename KeyType, typename ValueType>
	jobject ConvertToJava(JNIEnv* env, const std::pair<KeyType, ValueType>& source);

	template<typename KeyType, typename ValueType>
	jobject ConvertToJavaMap(JNIEnv* env, const std::map<KeyType, ValueType>& source);

	template<typename KeyType, typename ValueType>
	jobject ConvertToJava(JNIEnv* env, const std::map<KeyType, ValueType>& source);

	template<typename SourceType>
	jobject NotImplemented(JNIEnv* env, const SourceType& source) {
		typedef typename ConversionIsNotSupported<SourceType>::ASSERT_illegal_call_to_unimplemented_conversion_method_for_source_type compile_error;
		return (jobject)0;
	}

};
#endif
