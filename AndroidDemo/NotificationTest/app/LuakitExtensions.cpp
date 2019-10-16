#include "jni/JniEnvWrapper.h"
#include "com_common_luakit_LuaHelper.h"
#include "com_common_luakit_LuaNotificationListener.h"
#include "com_common_luakit_NotificationHelper.h"
class JniEnvWrapper;
static JniEnvWrapper* dummy; // just to satisfy this stupid android linker
void toto()
{
Java_com_common_luakit_LuaHelper_startLuaKitNative(0,0,0);
Java_com_common_luakit_LuaNotificationListener_nativeNewNotificationListener(0,0);
Java_com_common_luakit_NotificationHelper_postNotificationNative(0,0,0,0);
}

class LuakitExtension;
extern LuakitExtension TheNotifyExtension;
extern LuakitExtension TheThreadExtension;
extern LuakitExtension* ExtensionsList [] =
{
    &TheNotifyExtension,
    &TheThreadExtension,
	0
};
