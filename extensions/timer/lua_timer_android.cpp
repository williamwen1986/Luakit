extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "common/business_runtime.h"
#include "common/base_lambda_support.h"
#include "lua_timer.h"
#include "lua_helpers.h"
#include "lua_timer.h"
#include "base/android/jni_generator/jni_generator_helper.h"
#include "java_weak_ref.h"

const char kTimerUtilClassPath[] = "com/common/luakit/TimerUtil";
// Leaking this jclass as we cannot use LazyInstance from some threads.
jclass g_TimerUtil_clazz = NULL;

struct AndroidTimer
{
    java_weak_ref * ref;
    int type;
};

class TimerExtension : LuakitExtension
{
public:
    /* ctor */ TimerExtension()
    {
        license = MIT;
        extensionName = "Timer";
        needChromium = true;
    }
    virtual void LuaOpen(lua_State* L)
    {
        //if (!isOpen)
        {
            if (! isOpen)
            {
                isOpen = true;
                extern bool RegisterTimerUtil();
                RegisterTimerUtil();
            }
            luaopen_timer(L);
        }
    }
} TheTimerExtension;



static base::subtle::AtomicWord g_createTimer = 0;
static jobject createTimer(JNIEnv* env) {
  CHECK_CLAZZ(env, g_TimerUtil_clazz,
      g_TimerUtil_clazz, NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, g_TimerUtil_clazz,
      "createTimer",
      "()Ljava/util/Timer;",
      &g_createTimer);
  jobject timer = env->CallStaticObjectMethod(g_TimerUtil_clazz,method_id);
  jni_generator::CheckException(env);
  return timer;
}

static base::subtle::AtomicWord g_setNativeRefs = 0;
static void setNativeRefs(JNIEnv* env, jobject timer, jlong ref) {
  BusinessThreadID now_thread_identifier;
  BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
  CHECK_CLAZZ(env, g_TimerUtil_clazz,
      g_TimerUtil_clazz, NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, g_TimerUtil_clazz,
      "setNativeRefs",
      "(Ljava/util/Timer;JI)V",
      &g_setNativeRefs);
  env->CallStaticVoidMethod(g_TimerUtil_clazz,method_id,timer,ref,now_thread_identifier);
  jni_generator::CheckException(env);
}

static base::subtle::AtomicWord g_startTimer = 0;
static void startTimer(JNIEnv* env, jobject timer, jint type,jlong period) {
  CHECK_CLAZZ(env, g_TimerUtil_clazz,
      g_TimerUtil_clazz, NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, g_TimerUtil_clazz,
      "startTimer",
      "(Ljava/util/Timer;IJ)V", 
      &g_startTimer);
  env->CallStaticVoidMethod(g_TimerUtil_clazz,method_id,timer,type,period);
  jni_generator::CheckException(env);
}


static base::subtle::AtomicWord g_stopTimer = 0;
static void stopTimer(JNIEnv* env, jobject timer) {
  CHECK_CLAZZ(env, g_TimerUtil_clazz,
      g_TimerUtil_clazz, NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, g_TimerUtil_clazz,
      "stopTimer",
      "(Ljava/util/Timer;)V",  
      &g_stopTimer);
  env->CallStaticVoidMethod(g_TimerUtil_clazz,method_id,timer);
  jni_generator::CheckException(env);
}

static base::subtle::AtomicWord g_releaseTimer = 0;
static void releaseTimer(JNIEnv* env, jobject timer) {
  CHECK_CLAZZ(env, g_TimerUtil_clazz,
      g_TimerUtil_clazz, NULL);
  jmethodID method_id =
      base::android::MethodID::LazyGet<
      base::android::MethodID::TYPE_STATIC>(
      env, g_TimerUtil_clazz,
      "releaseTimer",
      "(Ljava/util/Timer;)V",  
      &g_releaseTimer);
  env->CallStaticVoidMethod(g_TimerUtil_clazz,method_id,timer);
  jni_generator::CheckException(env);
}

static void timerCallJni(jlong ref, jint threadId) {
    BusinessThread::PostTask(threadId, FROM_HERE, base::BindLambda([=](){
        lua_State * state = BusinessThread::GetCurrentThreadLuaState();
        pushUserdataInWeakTable(state,(jobject)ref);
        if (!lua_isnil(state, -1)) {
            lua_getfield(state, -1, "callback");
            if (lua_isfunction(state, -1)) {
                int err = lua_pcall(state, 0, 0, 0);
                if (err != 0) {
                    luaL_error(state,"lua timer callback error");
                }
            } else {
                lua_pop(state, 1);
            }
        }
        lua_pop(state, 1);               
    }));
}


static void timerCall(JNIEnv* env, jclass jcaller, jlong ref, jint threadId) {
    timerCallJni(ref, threadId);
}

static const JNINativeMethod kMethodsTimerUtil[] = {
    {   "timerCall",
        "(JI)V",
        reinterpret_cast<void*>(timerCall) }
};

extern bool RegisterTimerUtil() {
  JNIEnv* env = base::android::AttachCurrentThread();
  g_TimerUtil_clazz = reinterpret_cast<jclass>(env->NewGlobalRef(
      base::android::GetClass(env, kTimerUtilClassPath).obj()));
  const int kMethodsTimerUtilSize = arraysize(kMethodsTimerUtil);

  if (env->RegisterNatives(g_TimerUtil_clazz,
                           kMethodsTimerUtil,
                           kMethodsTimerUtilSize) < 0) {
    jni_generator::HandleRegistrationError(
        env, g_TimerUtil_clazz, __FILE__);
    return false;
  }
  return true;
}

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
    AndroidTimer *instanceUserdata = (AndroidTimer *)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    JNIEnv* env = base::android::AttachCurrentThread();
    jobject timer = instanceUserdata->ref->obj();
    releaseTimer(env, timer);
    delete instanceUserdata->ref;
    instanceUserdata->ref = NULL;
    return 0;
}

static int start(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    assert(lua_gettop(L) == 3);
    AndroidTimer *instanceUserdata = (AndroidTimer *)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    jobject timer = instanceUserdata->ref->obj();
    int milliseconds = luaL_checkint(L, 2);
    lua_remove(L, 2);
    lua_setfield(L, -2, "callback");
    JNIEnv* env = base::android::AttachCurrentThread();
    startTimer(env,timer,(jint)instanceUserdata->type,(jlong)milliseconds);
    END_STACK_MODIFY(L, 0)
    return 0;
}

static int reset(lua_State *L)
{
    AndroidTimer *instanceUserdata = (AndroidTimer *)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    jobject timer = instanceUserdata->ref->obj();
    JNIEnv* env = base::android::AttachCurrentThread();
    stopTimer(env, timer);
    return 0;
}

static int stop(lua_State *L)
{
    AndroidTimer *instanceUserdata = (AndroidTimer *)luaL_checkudata(L, 1, LUA_TIMER_METATABLE_NAME);
    jobject timer = instanceUserdata->ref->obj();
    JNIEnv* env = base::android::AttachCurrentThread();
    stopTimer(env, timer);
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

    JNIEnv* env = base::android::AttachCurrentThread();
    jobject timer = createTimer(env);
    java_weak_ref *weakRef = new java_weak_ref(timer);
    size_t nbytes = sizeof(AndroidTimer);
    AndroidTimer *instanceUserdata = (AndroidTimer *)lua_newuserdata(L, nbytes);
    instanceUserdata->ref = weakRef;
    instanceUserdata->type = type;

    setNativeRefs(env, timer, (jlong)timer);

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

