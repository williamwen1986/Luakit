extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "lua_notification_utility.h"
#include "common/base_lambda_support.h"
#include "tools/lua_helpers.h"
#include "JniLuaConvertor.h"
#include "JniEnvWrapper.h"
#include "JavaRefCountedWrapper.h"

extern void notification_call_to_lua(LuaNotificationListener * l, int type, const content::NotificationSource& source, const content::NotificationDetails& details)
{
    content::Details<void> content(details);
    void * d = content.ptr();
    jobject data = (jobject)d;

    BusinessThreadID from_thread_identifier;
    BusinessThread::GetCurrentThreadIdentifier(&from_thread_identifier);
    if (from_thread_identifier == l->threadId) {
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        pushUserdataInWeakTable(state,l);
        if (!lua_isnil(state, -1)) {
            lua_pushinteger(state, type);
            lua_gettable(state, -2);
            if (lua_isfunction(state, -1)) {
                if (data) {
                    JniEnvWrapper env;
                    object_fromjava(state, *env, data);
                    int err = lua_pcall(state, 1, 0, 0);
                    if (err != 0) {
                        luaL_error(state,"LuaNotificationListener::Observe call error %s", lua_tostring(state, -1));
                    }
                } else {
                    int err = lua_pcall(state, 0, 0, 0);
                    if (err != 0) {
                        luaL_error(state,"LuaNotificationListener::Observe call error %s", lua_tostring(state, -1));
                    }
                }
            } else {
                lua_pop(state, 1);
            }
        }
        lua_pop(state, 1);
    } else {
        scoped_refptr<JavaRefCountedWrapper> dataPtr;
        if (data)
        {
           dataPtr = make_javarefptr(data);
        }
        scoped_refptr<LuaNotificationListener> listener = make_scoped_refptr(l);
        BusinessThread::PostTask(l->threadId, FROM_HERE, base::BindLambda([=](){
            lua_State * state = BusinessThread::GetCurrentThreadLuaState();
            pushUserdataInWeakTable(state,listener.get());
            if (!lua_isnil(state, -1)) {
                lua_pushinteger(state, type);
                lua_gettable(state, -2);
                if (lua_isfunction(state, -1)) {
                    if (dataPtr.get()) {
                        JniEnvWrapper env;
                        object_fromjava(state, *env, dataPtr.get()->obj());
                        int err = lua_pcall(state, 1, 0, 0);
                        if (err != 0) {
                            luaL_error(state,"LuaNotificationListener::Observe call error %s", lua_tostring(state, -1));
                        }
                    } else {
                        int err = lua_pcall(state, 0, 0, 0);
                        if (err != 0) {
                            luaL_error(state,"LuaNotificationListener::Observe call error %s", lua_tostring(state, -1));
                        }
                    }
                } else {
                    lua_pop(state, 1);
                }
            }
            lua_pop(state, 1);
        }));
    }
}



