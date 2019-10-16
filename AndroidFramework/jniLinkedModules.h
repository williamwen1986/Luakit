// Hack to force the link-editor to link-edit jni.
#include "com_common_luakit_LuaHelper.h"
#include "com_common_luakit_LuaNotificationListener.h"
#include "com_common_luakit_NotificationHelper.h"
#include "common/notification_registrar.h"
#include "base/threading/thread_collision_warner.h"
void dummyFn()
{
    content::NotificationRegistrar dummy;
    base::DCheckAsserter dummy2;
    Java_com_common_luakit_LuaHelper_startLuaKitNative(0,0,0);
    Java_com_common_luakit_LuaNotificationListener_nativeNewNotificationListener(0,0);
    Java_com_common_luakit_NotificationHelper_postNotificationNative(0,0,0,0);
}


