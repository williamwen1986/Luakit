#ifdef __cplusplus
extern "C" {
#endif
#include "lauxlib.h"
#include "lapi.h"
#ifdef __cplusplus
}
#endif
#include "lua_helpers.h"
#include "JniCallbackHelper.h"
#include "JavaRefCountedWrapper.h"
#include "JniEnvWrapper.h"
#include "JniLuaConvertor.h"

#define LUA_JNI_CALLBACK_METATABLE_NAME "lua.jnicallback"

static int __gc(lua_State *L);
static int __call(lua_State *L);

static int __gc(lua_State *L)
{
    JavaRefCountedWrapper **instanceUserdata = (JavaRefCountedWrapper **)luaL_checkudata(L, 1, LUA_JNI_CALLBACK_METATABLE_NAME);
    delete *instanceUserdata;
    return 0;
}

static int __call(lua_State *L)
{
    assert(lua_gettop(L) == 2);
    JavaRefCountedWrapper **instanceUserdata = (JavaRefCountedWrapper **)luaL_checkudata(L, 1, LUA_JNI_CALLBACK_METATABLE_NAME);
    JniEnvWrapper env;
	if (env->IsSameObject((*instanceUserdata)->obj(), NULL)) {
		return 0;
	}
	jclass clazz = env->GetObjectClass((*instanceUserdata)->obj());
    jmethodID methodid = env->GetMethodID(clazz, "onResult", "(Ljava/lang/Object;)V");
	env->PushLocalFrame(0);
	env->CallVoidMethod(
		(*instanceUserdata)->obj(),
		methodid,
		object_copyToJava(L, *env ,2)
		);
	env->PopLocalFrame(NULL);
    return 0;
}

static const struct luaL_Reg metaFunctions[] = {
    {"__gc", __gc},
    {"__call", __call},
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {NULL, NULL}
};

extern int luaopen_jni_callback(lua_State* L)
{
	BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_JNI_CALLBACK_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_JNI_CALLBACK_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}

extern void push_jni_block(lua_State* L, jobject callback)
{
	luaL_getmetatable(L, LUA_JNI_CALLBACK_METATABLE_NAME);
    if (lua_isnil(L, -1)) {
        luaopen_jni_callback(L);
    }
    lua_pop(L, 1);
    JavaRefCountedWrapper * wrapper = new JavaRefCountedWrapper(callback);
    size_t nbytes = sizeof(JavaRefCountedWrapper *);
    JavaRefCountedWrapper **instanceUserdata = (JavaRefCountedWrapper **)lua_newuserdata(L, nbytes);
	*instanceUserdata = wrapper;
	luaL_getmetatable(L, LUA_JNI_CALLBACK_METATABLE_NAME);
    lua_setmetatable(L, -2);
}