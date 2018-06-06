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