extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "common/business_runtime.h"
#include "common/base_lambda_support.h"
#include "lua_timer.h"
#include "lua_helpers.h"
#include "base/timer/timer.h"

static int __newindex(lua_State *L)
{
    BEGIN_STACK_MODIFY(L);
    // Add value to the userdata's environment table.
    lua_getfenv(L, 1);
    lua_insert(L, 2);
    lua_rawset(L, 2);
    END_STACK_MODIFY(L, 0);
    return 0;
}

static int __gc(lua_State *L)
{
    base::Timer **instanceUserdata = (base::Timer **)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    base::Timer *timer = *instanceUserdata;
    if (timer->IsRunning()){
        timer->Stop();
    }
    delete timer;
    return 0;
}

static int start(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    assert(lua_gettop(L) == 3);
    base::Timer **instanceUserdata = (base::Timer **)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    base::Timer *timer = *instanceUserdata;
    int milliseconds = luaL_checkint(L, 2);
    lua_remove(L, 2);
    lua_setfield(L, -2, "callback");
    
    timer->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(milliseconds), base::BindLambda([=](){
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        pushUserdataInWeakTable(state,timer);
        if (!lua_isnil(state, -1)) {
            lua_getfield(state, -1, "callback");
            if (lua_isfunction(L, -1)) {
                int err = lua_pcall(state, 0, 0, 0);
                if (err != 0) {
                    luaL_error(L,"lua timer callback error");
                }
            } else {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }));
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int reset(lua_State *L)
{
    base::Timer **instanceUserdata = (base::Timer **)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    base::Timer *timer = *instanceUserdata;
    timer->Reset();
    return 0;
}

static int stop(lua_State *L)
{
    base::Timer **instanceUserdata = (base::Timer **)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    base::Timer *timer = *instanceUserdata;
    timer->Stop();
    return 0;
}

static int __index(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    lua_getfenv(L, -2);
    lua_pushvalue(L, -2);
    lua_rawget(L, 3);
    if (!lua_isnil(L, -1)) {
        lua_insert(L, 1);
        lua_settop(L, 1);
        return 1;
    } else {
        lua_settop(L, 2);
    }
    const char *c = lua_tostring(L, 2);
    lua_settop(L, 0);
    luaL_getmetatable(L, LUA_TIMER_METATABLE_NAME);
    lua_getfield(L, -1, c);
    lua_remove(L, 1);
    END_STACK_MODIFY(L, 1)
    return 1;
}

static int createTimer(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    assert(lua_gettop(L) == 1);
    int type = luaL_checkint(L, 1);//0-onetime,1-repeat
    lua_remove(L, 1);
    base::Timer *timer = NULL;
    size_t nbytes = sizeof(base::Timer *);
    base::Timer **instanceUserdata = (base::Timer **)lua_newuserdata(L, nbytes);
    if (type == 0) {
        timer = new base::OneShotTimer<base::Timer>();
    } else {
        timer = new base::RepeatingTimer<base::Timer>();
    }
    *instanceUserdata = timer;
    // set the metatable
    luaL_getmetatable(L, LUA_TIMER_METATABLE_NAME);
    lua_setmetatable(L, -2);
    
    // give it a nice clean environment
    lua_newtable(L);
    lua_setfenv(L, -2);
    
    pushWeakUserdataTable(L);
    lua_pushlightuserdata(L, timer);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    
    END_STACK_MODIFY(L, 1)
    return 1;
}

static const struct luaL_Reg metaFunctions[] = {
    {"__index", __index},
    {"__newindex", __newindex},
    {"__gc", __gc},
    {"start", start},
    {"reset", reset},
    {"stop", stop},
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"createTimer", createTimer},
    {NULL, NULL}
};

extern int luaopen_timer(lua_State* L)
{
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_TIMER_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_TIMER_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}

