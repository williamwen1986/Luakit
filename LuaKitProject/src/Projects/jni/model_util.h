#ifndef ANDROID_MODEL_UTIL_H
#define ANDROID_MODEL_UTIL_H

#include "base/logging.h"
#include <jni.h>
#include "JNIConversionDecl.h"
#include "JNIConversionImpl.h"
#include "base/memory/ref_counted.h"

namespace util {
template <typename T>
jlong nativeCopy(jlong jhandle) {
    scoped_refptr<T>* pointer = (scoped_refptr<T>*)jhandle;

    if (pointer == nullptr) {
        LOG(ERROR) << "native copy " << typeid(pointer->get()).name() << ": handle = 0";
    }

    scoped_refptr<T>* pointer_copy = new scoped_refptr<T>(*pointer);

    return (jlong)pointer_copy;
}

template <typename T>
void nativeFree(jlong jhandle) {
    scoped_refptr<T>* pointer = (scoped_refptr<T>*)jhandle;
    if (pointer == nullptr) {
        LOG(ERROR) << "native free " << typeid(pointer->get()).name() << ": handle = 0";
    }

    if (pointer != 0) {
        delete pointer;
    }
}

}

#endif //ANDROID_MODEL_UTIL_H
