#include <com_common_luakit_LuaHelper.h>
#include "base/logging.h"
#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "common/business_main_delegate.h"
#include "common/business_runtime.h"
#include "base/android/base_jni_registrar.h"
#include "base/android/jni_android.h"
#include "JniEnvWrapper.h"
#include "base/thread_task_runner_handle.h"
#include "common/base_lambda_support.h"
#include "tools/lua_helpers.h"
#include "JniLuaConvertor.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"

JNIEXPORT void JNICALL Java_com_common_luakit_LuaHelper_startLuaKitNative
  (JNIEnv *env, jclass c, jobject context)
{
	base::android::InitApplicationContext(env, base::android::ScopedJavaLocalRef<jobject>(env, context));
  	LOG(ERROR) << "nativeNewObject support ";
  	setXXTEAKeyAndSign("2dxLua", strlen("2dxLua"), "XXTEA", strlen("XXTEA"));
    BusinessMainDelegate* delegate(new BusinessMainDelegate());
    BusinessRuntime* Business_runtime(BusinessRuntime::Create());
    Business_runtime->Initialize(delegate);
    Business_runtime->Run();
}

void pushLuaModule(std::string moduleName)
{
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    std::string lua = "TEM_OC_OBJECT = require('"+ moduleName +"')"; 
    doString(L, lua.c_str());
    lua_getglobal(L, "TEM_OC_OBJECT");
    lua_pushnil(L);
    lua_setglobal(L, "TEM_OC_OBJECT");
    END_STACK_MODIFY(L, 1)
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        int err = lua_pcall(L, 0, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName,jobject p1)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,p1);
        int err = lua_pcall(L, 1, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2Ljava_lang_Object_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName,jobject p1, jobject p2)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,p1);
        object_fromjava(L, env ,p2);
        int err = lua_pcall(L, 2, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName, jobject p1, jobject p2, jobject p3)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,p1);
        object_fromjava(L, env ,p2);
        object_fromjava(L, env ,p3);
        int err = lua_pcall(L, 3, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName, jobject p1,jobject p2,jobject p3,jobject p4)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,p1);
        object_fromjava(L, env ,p2);
        object_fromjava(L, env ,p3);
        object_fromjava(L, env ,p4);
        int err = lua_pcall(L, 4, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

JNIEXPORT jobject JNICALL Java_com_common_luakit_LuaHelper_callLuaFunction__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2Ljava_lang_Object_2
  (JNIEnv *env, jclass, jstring moduleName, jstring methodName,jobject p1,jobject p2,jobject p3,jobject p4,jobject p5)
{
    const char* module = env->GetStringUTFChars((jstring)moduleName, NULL);
    const char* method = env->GetStringUTFChars((jstring)methodName, NULL);
    lua_State * L = BusinessThread::GetCurrentThreadLuaState();
    BEGIN_STACK_MODIFY(L)
    pushLuaModule(module);
    lua_pushstring(L, method);
    lua_rawget(L, -2);
    jobject ret = NULL;
    if (lua_isfunction(L, -1)) {
        object_fromjava(L, env ,p1);
        object_fromjava(L, env ,p2);
        object_fromjava(L, env ,p3);
        object_fromjava(L, env ,p4);
        object_fromjava(L, env ,p5);
        int err = lua_pcall(L, 5, 1, 0);
        if (err != 0) {
            luaError(L,"callLuaFunction call error");
        } else {
            ret = object_copyToJava(L, env,-1);
        }
    } else {
         luaError(L,"callLuaFunction call error no such function");
    }
    END_STACK_MODIFY(L, 0)
    env->ReleaseStringUTFChars((jstring)moduleName, module);
    env->ReleaseStringUTFChars((jstring)methodName, method);
    return ret;
}

jint JNI_OnLoad(JavaVM* jvm, void* reserved) {
    JniEnvWrapper env(jvm);
    JNIEnv * envp = env;

    LOG(INFO) << "JNI_Onload start";
    //初始化chromiumbase库的相关东西
    base::android::InitVM(jvm);
    if (!base::android::RegisterJni(env)) {
        LOG(ERROR)<<"base::android::RegisterJni(env) error";
        return -1;
    }
    CommandLine::Init(0, nullptr);

    LOG(INFO) << "JNI_Onload end";
    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif