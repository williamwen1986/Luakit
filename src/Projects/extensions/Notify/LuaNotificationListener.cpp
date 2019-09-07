extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "LuaNotificationListener.h"
#include "lua_notify.h"
#include "serialize.h"
#include "tools/lua_helpers.h"
#include "common/base_lambda_support.h"
#include "lua_notification_utility.h"

LuaNotificationListener::LuaNotificationListener(){
    BusinessThread::GetCurrentThreadIdentifier(&threadId);
}

LuaNotificationListener::~LuaNotificationListener(){
    LOG(INFO) << "~LuaNotificationListener ";
}

void LuaNotificationListener::AddObserver(int type) {
    LOG(INFO) << "native add notification observer. type = " << type;
    registrar_.Add(this, type, content::NotificationService::AllSources());
}

void LuaNotificationListener::RemoveObserver(int type) {
    LOG(INFO) << "native remove notification observer. type = " << type;
    registrar_.Remove(this, type, content::NotificationService::AllSources());
}

void LuaNotificationListener::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) {
    content::Source<Notification::sourceType> s(source);
    Notification::sourceType sourceType = (*s.ptr());
    switch (sourceType) {
        case Notification::LUA :
            {
                block * params = NULL;
                content::Details<lua_State> d(details);
                if(d.ptr()!=NULL) {
                    std::list<thread::CallbackContext *> callbackContexts;
                    params = seri_pack(d.ptr(),callbackContexts,1);
                }
                BusinessThreadID from_thread_identifier;
                BusinessThread::GetCurrentThreadIdentifier(&from_thread_identifier);
                if (from_thread_identifier == threadId) {
                    pushUserdataInWeakTable(d.ptr(),this);
                    if (!lua_isnil(d.ptr(), -1)) {
                        lua_pushinteger(d.ptr(), type);
                        lua_gettable(d.ptr(), -2);
                        if (lua_isfunction(d.ptr(), -1)) {
                            if (params) {
                                lua_pushcfunction(d.ptr(), seri_unpack);
                                lua_pushlightuserdata(d.ptr(), params);
                                int err = lua_pcall(d.ptr(), 1, 1, 0);
                                
                                if (err != 0) {
                                    luaL_error(d.ptr(),"LuaNotificationListener::Observe seri_unpack call error");
                                } else {
                                    err = lua_pcall(d.ptr(), 1, 0, 0);
                                    if (err != 0) {
                                        luaL_error(d.ptr(),"LuaNotificationListener::Observe call error %s", lua_tostring(d.ptr(), -1));
                                    }
                                }
                            } else {
                                int err = lua_pcall(d.ptr(), 0, 0, 0);
                                if (err != 0) {
                                    luaL_error(d.ptr(),"LuaNotificationListener::Observe call error %s", lua_tostring(d.ptr(), -1));
                                }
                            }
                        } else {
                            lua_pop(d.ptr(), 1);
                        }
                    }
                    lua_pop(d.ptr(), 1);
                } else {
                    scoped_refptr<LuaNotificationListener> l = make_scoped_refptr(this);
                    BusinessThread::PostTask(threadId, FROM_HERE, base::BindLambda([=](){
                        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
                        pushUserdataInWeakTable(state,l.get());
                        if (!lua_isnil(state, -1)) {
                            lua_pushinteger(state, type);
                            lua_gettable(state, -2);
                            if (lua_isfunction(state, -1)) {
                                if (params) {
                                    lua_pushcfunction(state, seri_unpack);
                                    lua_pushlightuserdata(state, params);
                                    int err = lua_pcall(state, 1, 1, 0);

                                    if (err != 0) {
                                        luaL_error(state,"LuaNotificationListener::Observe seri_unpack call error");
                                    } else {
                                        err = lua_pcall(state, 1, 0, 0);
                                        if (err != 0) {
                                            luaL_error(state,"LuaNotificationListener::Observe call error %s", lua_tostring(state, -1));
                                        }
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
            break;
        default:
            notification_call_to_lua(this, type , source, details);
            break;
    }
    
}
