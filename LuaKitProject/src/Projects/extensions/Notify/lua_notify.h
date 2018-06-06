#pragma once
extern "C" {
#include "lua.h"
}
#include "LuaNotificationListener.h"
#define LUA_NOTIFICATION_METATABLE_NAME "lua.notification"

class Notification {
public:
    enum sourceType {
        ANDROID_SYS,
        IOS_SYS,
        LUA
    };
};

class listenerWrapper{
public:
    scoped_refptr<LuaNotificationListener> listener;
};

extern int luaopen_notification(lua_State* L);

