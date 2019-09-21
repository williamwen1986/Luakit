#ifndef JAVAREFCOUNTEDWRAPPER_H
#define JAVAREFCOUNTEDWRAPPER_H
#include "JniEnvWrapper.h"
#include "base/memory/ref_counted.h"
#include "base/logging.h"

class JavaRefCountedWrapper : public base::RefCounted<JavaRefCountedWrapper> {
public:
	JavaRefCountedWrapper(jobject obj) {
		JniEnvWrapper env;
		m_obj = env->NewGlobalRef(obj);
	}
	JavaRefCountedWrapper(JNIEnv* env, jobject obj) {
		m_obj = env->NewGlobalRef(obj);
	}
	~JavaRefCountedWrapper() {
		JniEnvWrapper env;
		env->DeleteGlobalRef(m_obj);
	}
	jobject obj() {
		return m_obj;
	}
	static scoped_refptr<JavaRefCountedWrapper> convenience_make_javarefptr(jobject obj) {
		return scoped_refptr<JavaRefCountedWrapper>(new JavaRefCountedWrapper(obj));
	}
	static scoped_refptr<JavaRefCountedWrapper> convenience_make_javarefptr(JNIEnv* env, jobject obj) {
		return scoped_refptr<JavaRefCountedWrapper>(new JavaRefCountedWrapper(env, obj));
	}
private:
	jobject m_obj;
};

#define make_javarefptr(obj) JavaRefCountedWrapper::convenience_make_javarefptr(obj)
typedef scoped_refptr<JavaRefCountedWrapper> jobjectrefptr;
#endif
