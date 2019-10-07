#pragma once
extern "C" {
#include "lua.h"
}
#include <map>
#include <list>
#include <mutex>
#include "base/memory/ref_counted.h"
#include "common/common/business_client_thread.h"
#define LUA_THREAD_METATABLE_NAME "lua_thread"
#define LUA_CALLBACK_METATABLE_NAME "lua_callback"


namespace thread {
class ThreadContext : public base::RefCountedThreadSafe<ThreadContext> {
    
};

//用于跟踪callback传递的流程
struct CallbackInfo {
    BusinessThreadID threadId;
    std::string fromModule;
    std::string fromMethod;
};
    
typedef std::map<BusinessThreadID,thread::CallbackInfo>::iterator ITER;
    
struct CallbackContext {
    BusinessThreadID fromThread;
        //callback可能被抛到多个线程，当所有线程的callback都被释放时，callback引用的lua function也可以被回收了
    std::map<BusinessThreadID,CallbackInfo> toTheadList;
    std::recursive_mutex  lock;
};

}
extern int luaopen_thread(lua_State* L);
extern int luaopen_callback(lua_State* L);

