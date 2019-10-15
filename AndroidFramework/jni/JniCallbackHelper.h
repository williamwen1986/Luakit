#ifndef JNIENVCALLBACK_H_
#define JNIENVCALLBACK_H_
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#ifdef __cplusplus
}
#endif

extern int luaopen_jni_callback(lua_State* L);
extern void push_jni_block(lua_State* L, jobject callback);

#endif /* JNIENVCALLBACK_H_ */
