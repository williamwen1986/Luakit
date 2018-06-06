#include "java_weak_ref.h"
#include "JniEnvWrapper.h"
#include "base/logging.h"

java_weak_ref::java_weak_ref(jobject obj) : weak_ref_(NULL) {
    JniEnvWrapper env;

    if (env->IsSameObject(obj, NULL)) {
        LOG(INFO) << "java object is null";
    } else {
        weak_ref_ = env->NewWeakGlobalRef(obj);
        
        if(env->IsSameObject(weak_ref_, NULL)) {
            LOG(ERROR) << "new weak global ref failed";
        }
    }
}

java_weak_ref::~java_weak_ref() {
    JniEnvWrapper env;

    if (!env->IsSameObject(weak_ref_, NULL)) {
        env->DeleteWeakGlobalRef(weak_ref_);
    }
}