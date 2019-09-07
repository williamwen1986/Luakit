#ifndef LTASK_SERIALIZE_H
#define LTASK_SERIALIZE_H
extern "C" {
#include <lua.h>
}
#include "lua_thread.h"
#define BLOCK_SIZE 128

struct block {
    struct block * next;
    char buffer[BLOCK_SIZE];
};

extern int seri_unpack(lua_State *L);
extern struct block * seri_pack(lua_State *L ,std::list<thread::CallbackContext *> & callbackContext,  int count = -1);

#endif
