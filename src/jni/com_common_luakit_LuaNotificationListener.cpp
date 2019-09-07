#include "com_common_luakit_LuaNotificationListener.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "JNIConversionImpl.h"
#include "model_util.h"
#include "JNIConversionDecl.h"

namespace JNIConversion {
  IMPLEMENT_JAVAOBJECT_CONVERSION_MODEL(scoped_refptr<NotificationListenerImpl>, LogicModel::NotificationListener)
}

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jlong JNICALL Java_com_common_luakit_LuaNotificationListener_nativeNewNotificationListener
        (JNIEnv *, jobject thiz) {
    return (jlong)(new scoped_refptr<NotificationListenerImpl>(new NotificationListenerImpl(thiz)));
  }

/*
 * Class:     com_common_luakit_LuaNotificationListener
 * Method:    nativeAddObserver
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_common_luakit_LuaNotificationListener_nativeAddObserver
(JNIEnv *env, jobject thiz, jint type) {
    scoped_refptr<NotificationListenerImpl> listener;
    JNIConversion::ConvertToNative(env, thiz, listener);
    DCHECK(listener != nullptr);

    listener->AddObserver((int)type);
  }

/*
 * Class:     com_common_luakit_LuaNotificationListener
 * Method:    nativeRemoveObserver
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_common_luakit_LuaNotificationListener_nativeRemoveObserver
(JNIEnv *env, jobject thiz, jint type) {
    scoped_refptr<NotificationListenerImpl> listener;
    JNIConversion::ConvertToNative(env, thiz, listener);
    DCHECK(listener != nullptr);

    LOG(INFO) << "notification nativeRemoveObserver: " << type;
    listener->RemoveObserver((int)type);
	  
  }

#ifdef __cplusplus
}
#endif