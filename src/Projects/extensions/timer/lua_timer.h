#pragma once
extern "C" {
#include "lua.h"
}
#if defined(OS_ANDROID)
#include <jni.h>
#include "base/android/jni_android.h"
#endif

#define LUA_TIMER_METATABLE_NAME "lua_timer"
extern int luaopen_timer(lua_State* L);

#if defined(OS_ANDROID)
extern bool RegisterTimerUtil(JNIEnv* env);
#endif

