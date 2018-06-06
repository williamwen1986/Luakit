#include <com_common_luakit_Demo.h>
#include "common/business_runtime.h"
#include "lua_helpers.h"
#include "JniLuaConvertor.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"

static void pushLuaObject()
{
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    std::string lua = "TEM_OC_OBJECT = require('WeatherManager')"; 
    doString(L, lua.c_str());
    lua_getglobal(L, "TEM_OC_OBJECT");
    lua_pushnil(L);
    lua_setglobal(L, "TEM_OC_OBJECT");
    END_STACK_MODIFY(L, 1)
}

JNIEXPORT jobjectArray JNICALL Java_com_common_luakit_Demo_getWeatherNative
  (JNIEnv *env, jclass c)
{
	lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaObject();
    lua_pushstring(L, "getWeather");
    lua_rawget(L, -2);
    jobjectArray ret = NULL;
    if (lua_isfunction(L, -1)) {
        int err = lua_pcall(L, 0, 1, 0);
        if (err != 0) {
            luaError(L,"getWeather call error");
        } else {
            ret = (jobjectArray)object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"getWeather call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    return ret;
}

JNIEXPORT void JNICALL Java_com_common_luakit_Demo_loadWeatherNative
  (JNIEnv *env, jclass c, jobject callback)
 {
 	lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaObject();
    lua_pushstring(L, "loadWeather");
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,callback);
        int err = lua_pcall(L, 1, 0, 0);
        if (err != 0) {
            luaError(L,"loadWeather call error");
        }
    } else {
        luaError(L,"loadWeather call error no such function");
    }
    END_STACK_MODIFY(L, 0)
 }

JNIEXPORT void JNICALL Java_com_common_luakit_Demo_threadTestNative
        (JNIEnv *, jclass)
{
	lua_State * state = BusinessThread::GetCurrentThreadLuaState();
    luaL_dostring(state, "require('thread_test').fun1()");
}

JNIEXPORT void JNICALL Java_com_common_luakit_Demo_notificationTestNative
        (JNIEnv *, jclass)
{
	lua_State * state = BusinessThread::GetCurrentThreadLuaState();
    luaL_dostring(state, "require('notification_test').test()");
}

JNIEXPORT void JNICALL Java_com_common_luakit_Demo_asynSocketTestNative
        (JNIEnv *, jclass)
{
	lua_State * state = BusinessThread::GetCurrentThreadLuaState();
    luaL_dostring(state, "require('async_socket_test').test()");
}

JNIEXPORT void JNICALL Java_com_common_luakit_Demo_dbTestNative
        (JNIEnv *, jclass)
{
	lua_State * state = BusinessThread::GetCurrentThreadLuaState();
    luaL_dostring(state, "require('db_test').test()");
}

#ifdef __cplusplus
}
#endif