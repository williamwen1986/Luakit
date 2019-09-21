#ifndef JNIENVLUA_H_
#define JNIENVLUA_H_
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif

extern void object_fromjava(lua_State *L, JNIEnv *env, jobject o);

extern jobject object_copyToJava(lua_State *L, JNIEnv *env, int stackIndex);

#endif /* JNIENVLUA_H_ */
