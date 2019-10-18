#include "com_common_luakit_NotificationHelper.h"
#include "notification_service.h"
#include "notification_details.h"
#include "notification_source.h"
#include "lua_notify.h"
#include "JniLuaConvertor.h"
#include "base/logging.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"
/*
 * Class:     com_common_luakit_NotificationHelper
 * Method:    postNotificationNative
 * Signature: (ILjava/lang/Object;)V
 */
JNIEXPORT jlong JNICALL Java_com_common_luakit_NotificationHelper_postNotificationNative
        (JNIEnv * env, jclass c, jint type, jobject data)
{
    Notification::sourceType s = Notification::ANDROID_SYS;
    content::NotificationService::current()->Notify(type,content::Source<Notification::sourceType>(&s),content::Details<void>((void *)data));
    // Patch [LARPOUX] : the return value was missing
    return 0L;
}

#ifdef __cplusplus
}
#endif
