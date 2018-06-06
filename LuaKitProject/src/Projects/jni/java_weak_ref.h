#ifndef ANDROID_JAVA_WEAK_REF_H
#define ANDROID_JAVA_WEAK_REF_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

class java_weak_ref {
public:
    java_weak_ref(jobject obj);
    virtual ~java_weak_ref();
    jobject obj() { return weak_ref_;}

protected:
    jweak weak_ref_;
};

#ifdef __cplusplus
}
#endif
#endif //ANDROID_JAVA_WEAK_REF_H
