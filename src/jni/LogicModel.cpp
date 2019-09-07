#include "LogicModel.h"
#include "base/logging.h"

namespace LogicModel {
  static const char* NativeHandleField = "mNativeHandle";

  template<class T>
    void ConvertJavaToRefPtr(JNIEnv* env, jobject obj, const char* javaClassSig, scoped_refptr<T>& native) {
      if (env->IsSameObject(obj, 0)) {
          LOG(ERROR) << "ConvertJavaToRefPtr<" << javaClassSig << ">, obj = NULL";
          return;
      }

      JniEnvWrapper envw(env);
      long pointer = envw.GetLongField(obj, javaClassSig, LogicModel::NativeHandleField);
      if (pointer != 0) {
          native = *(scoped_refptr<T>*)pointer;
      } else {
          LOG(ERROR) << "ConvertJavaToRefPtr<" << javaClassSig << ">, handle = 0";
      }
    }

    template<class T>
    jobject ConvertRefPtrToJava(JNIEnv* env, const scoped_refptr<T>& native, const char* javaClassSig) {
      if (native == nullptr) {
          return NULL;
      }

      scoped_refptr<T>* pointer = new scoped_refptr<T>(native);
      JniEnvWrapper envw(env);
      jobject obj = envw.NewObject(javaClassSig, "(J)V", (jlong)pointer);

      return obj;
    }

  namespace NotificationListener {
      jobject ConvertToJava(JNIEnv* env, const scoped_refptr<NotificationListenerImpl>& native) {
          NOTREACHED();
          return nullptr;
      }

      void ConvertToNative(JNIEnv* env, jobject obj, scoped_refptr<NotificationListenerImpl>& native) {
          ConvertJavaToRefPtr(env, obj, classSig, native);
      }
  }

}
