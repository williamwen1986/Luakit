extern "C" {
#include "lua.h"
}
#include "tools/lua_helpers.h"
#include "lua_http_task.h"
#include "common/base_lambda_support.h"
#include "base/guid.h"
namespace network {
    
LuaHttpTask::LuaHttpTask(const std::string& name) : HttpTask(name){
}
    
LuaHttpTask::~LuaHttpTask() {
    
}

void LuaHttpTask::ProgressFeedBack(network::BaseConnection* connection, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow) {
    scoped_refptr<network::LuaHttpTask> ref = make_scoped_refptr(this);
    BusinessThread::PostTask(threadId, FROM_HERE, base::BindLambda([=](){
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        BEGIN_STACK_MODIFY(state);
        pushStrongUserdataTable(state);
        lua_pushlightuserdata(state, ref.get());
        lua_rawget(state, -2);
        lua_remove(state, -2);//盏顶是userdata
        if (lua_isuserdata(state, -1)) {
            lua_getfield(state, -1, "onProgress");//盏顶是onResponse函数
            if (lua_isnil(state, -1)) {
                lua_pop(state, 1);
            } else {
                //-1 function -2 userdata
                lua_newtable(state);
                //push dltotal
                lua_pushstring(state, "dltotal");
                lua_pushnumber(state, dltotal);
                lua_rawset(state, -3);
                
                //push dlnow
                lua_pushstring(state, "dlnow");
                lua_pushnumber(state, dlnow);
                lua_rawset(state, -3);
                
                //push ultotal
                lua_pushstring(state, "ultotal");
                lua_pushnumber(state, ultotal);
                lua_rawset(state, -3);
                
                //push ulnow
                lua_pushstring(state, "ulnow");
                lua_pushnumber(state, ulnow);
                lua_rawset(state, -3);
                
                lua_pcall(state, 1, 0, 0);
            }
            lua_pop(state, 1);
        } else {
            lua_pop(state, 1);
        }
        END_STACK_MODIFY(state, 0);
    }));
}

int LuaHttpTask::OnResponse(network::ProtocolErrorCode error_code,
                         const HTTP_HEADERS& h,
                         const std::string& resp, long http_code) {
    std::string response = resp;
    HTTP_HEADERS headers = h;
    scoped_refptr<network::LuaHttpTask> ref = make_scoped_refptr(this);
    BusinessThread::PostTask(threadId, FROM_HERE, base::BindLambda([=](){
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        BEGIN_STACK_MODIFY(state);
        pushStrongUserdataTable(state);
        lua_pushlightuserdata(state, ref.get());
        lua_rawget(state, -2);
        lua_remove(state, -2);//盏顶是userdata
        if (lua_isuserdata(state, -1)) {
            lua_getfield(state, -1, "onResponse");//盏顶是onResponse函数
            if (lua_isnil(state, -1)) {
                lua_pop(state, 1);
            } else {
                //-1 function -2 userdata
                lua_newtable(state);
                
                //push error_code
                lua_pushstring(state, "error_code");
                lua_pushnumber(state, error_code);
                lua_rawset(state, -3);
                
                //push headers
                lua_pushstring(state, "headers");
                lua_newtable(state);
                for(std::pair<std::string, std::string> p : h){
                    lua_pushstring(state, p.first.c_str());
                    lua_pushstring(state, p.second.c_str());
                    lua_rawset(state, -3);
                }
                lua_rawset(state, -3);
                
                //push resp
                lua_pushstring(state, "response");
                lua_pushstring(state, response.c_str());
                lua_rawset(state, -3);
                
                //push http_code
                lua_pushstring(state, "http_code");
                lua_pushnumber(state, http_code);
                lua_rawset(state, -3);
                
                lua_pcall(state, 1, 0, 0);
            }
            lua_pop(state, 1);
            //在强表清除userdata
            pushStrongUserdataTable(state);
            lua_pushlightuserdata(state, ref.get());
            lua_pushnil(state);
            lua_rawset(state, -3);
            lua_pop(state, 1);
        } else {
            lua_pop(state, 1);
        }
        END_STACK_MODIFY(state, 0)
    }));
    return 0;
}

}
    
