#include "com_common_luakit_NativeHandleHolder.h"
#include "notification_listener_impl.h"
#include "model_util.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_common_luakit_NativeHandleHolder
 * Method:    nativeFree
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_com_common_luakit_NativeHandleHolder_nativeFree
(JNIEnv *env , jobject, jlong jhandle, jint jtype) {
    if (jtype == 0){
        util::nativeFree<NotificationListenerImpl>(jhandle);
    }
}


#ifdef __cplusplus
}
#endif
