#ifndef __LSQLITE3_H__
#define __LSQLITE3_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

extern "C" int luaopen_lsqlite3(lua_State* L);   //lsqlite3.c 中的C函数，这里注册C函数
#endif