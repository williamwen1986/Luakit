extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
}
#include "lua_helpers.h"
#include "serialize.h"
#define TYPE_NIL 0
#define TYPE_BOOLEAN 1
// hibits 0 false 1 true
#define TYPE_NUMBER 2
// hibits 0 : 0 , 1: byte, 2:word, 4: dword, 6: qword, 8 : double
#define TYPE_NUMBER_ZERO 0
#define TYPE_NUMBER_BYTE 1
#define TYPE_NUMBER_WORD 2
#define TYPE_NUMBER_DWORD 4
#define TYPE_NUMBER_QWORD 6
#define TYPE_NUMBER_REAL 8

#define TYPE_USERDATA 3
#define TYPE_SHORT_STRING 4
// hibits 0~31 : len
#define TYPE_LONG_STRING 5
#define TYPE_TABLE 6
#define TYPE_FUNCTION 7

#define MAX_COOKIE 32
#define COMBINE_TYPE(t,v) ((t) | (v) << 3)

#define MAX_DEPTH 32

struct write_block {
	struct block * head;
	int len;
	struct block * current;
	int ptr;
};

struct read_block {
	char * buffer;
	struct block * current;
	int len;
	int ptr;
};

inline static struct block *
blk_alloc(void) {
	struct block *b = (struct block *)malloc(sizeof(struct block));
	b->next = NULL;
	return b;
}

inline static void
wb_push(struct write_block *b, const void *buf, int sz) {
	const char * buffer = (const char *)buf;
	if (b->ptr == BLOCK_SIZE) {
_again:
		b->current = b->current->next = blk_alloc();
		b->ptr = 0;
	}
	if (b->ptr <= BLOCK_SIZE - sz) {
		memcpy(b->current->buffer + b->ptr, buffer, sz);
		b->ptr+=sz;
		b->len+=sz;
	} else {
		int copy = BLOCK_SIZE - b->ptr;
		memcpy(b->current->buffer + b->ptr, buffer, copy);
		buffer += copy;
		b->len += copy;
		sz -= copy;
		goto _again;
	}
}

static void
wb_init(struct write_block *wb , struct block *b) {
	if (b==NULL) {
		wb->head = blk_alloc();
		wb->len = 0;
		wb->current = wb->head;
		wb->ptr = 0;
		wb_push(wb, &wb->len, sizeof(wb->len));
	} else {
		wb->head = b;
		int * plen = (int *)b->buffer;
		int sz = *plen;
		wb->len = sz;
		while (b->next) {
			sz -= BLOCK_SIZE;		
			b = b->next;
		}
		wb->current = b;
		wb->ptr = sz;
	}
}

static struct block *
wb_close(struct write_block *b) {
	b->current = b->head;
	b->ptr = 0;
	wb_push(b, &b->len, sizeof(b->len));
	b->current = NULL;
	return b->head;
}

static void
wb_free(struct write_block *wb) {
	struct block *blk = wb->head;
	while (blk) {
		struct block * next = blk->next;
		free(blk);
		blk = next;
	}
	wb->head = NULL;
	wb->current = NULL;
	wb->ptr = 0;
	wb->len = 0;
}

/*
static void
rball_init(struct read_block * rb, char * buffer) {
	rb->buffer = buffer;
	rb->current = NULL;
	uint8_t header[4];
	memcpy(header,buffer,4);
	rb->len = header[0] | header[1] <<8 | header[2] << 16 | header[3] << 24;
	rb->ptr = 4;
	rb->len -= rb->ptr;
}
*/
static int
rb_init(struct read_block *rb, struct block *b) {
	rb->buffer = NULL;
	rb->current = b;
	memcpy(&(rb->len),b->buffer,sizeof(rb->len));
	rb->ptr = sizeof(rb->len);
	rb->len -= rb->ptr;
	return rb->len;
}

static void *
rb_read(struct read_block *rb, void *buffer, int sz) {
	if (rb->len < sz) {
		return NULL;
	}

	if (rb->buffer) {
		int ptr = rb->ptr;
		rb->ptr += sz;
		rb->len -= sz;
		return rb->buffer + ptr;
	}

	if (rb->ptr == BLOCK_SIZE) {
		struct block * next = rb->current->next;
		free(rb->current);
		rb->current = next;
		rb->ptr = 0;
	}

	int copy = BLOCK_SIZE - rb->ptr;

	if (sz <= copy) {
		void * ret = rb->current->buffer + rb->ptr;
		rb->ptr += sz;
		rb->len -= sz;
		return ret;
	}

	char * tmp = (char *)buffer;

	memcpy(tmp, rb->current->buffer + rb->ptr,  copy);
	sz -= copy;
	tmp += copy;
	rb->len -= copy;

	for (;;) {
		struct block * next = rb->current->next;
		free(rb->current);
		rb->current = next;

		if (sz < BLOCK_SIZE) {
			memcpy(tmp, rb->current->buffer, sz);
			rb->ptr = sz;
			rb->len -= sz;
			return buffer;
		}
		memcpy(tmp, rb->current->buffer, BLOCK_SIZE);
		sz -= BLOCK_SIZE;
		tmp += BLOCK_SIZE;
		rb->len -= BLOCK_SIZE;
	}
}

static void
rb_close(struct read_block *rb) {
	while (rb->current) {
		struct block * next = rb->current->next;
		free(rb->current);
		rb->current = next;
	}
	rb->len = 0;
	rb->ptr = 0;
}

static inline void
wb_nil(struct write_block *wb) {
	int n = TYPE_NIL;
	wb_push(wb, &n, 1);
}

static inline void
wb_boolean(struct write_block *wb, int boolean) {
	int n = COMBINE_TYPE(TYPE_BOOLEAN , boolean ? 1 : 0);
	wb_push(wb, &n, 1);
}

static inline void
wb_integer(struct write_block *wb, lua_Integer v) {
	int type = TYPE_NUMBER;
	if (v == 0) {
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_ZERO);
		wb_push(wb, &n, 1);
	} else if (v != (int32_t)v) {
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_QWORD);
		int64_t v64 = v;
		wb_push(wb, &n, 1);
		wb_push(wb, &v64, sizeof(v64));
	} else if (v < 0) {
		int32_t v32 = (int32_t)v;
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_DWORD);
		wb_push(wb, &n, 1);
		wb_push(wb, &v32, sizeof(v32));
	} else if (v<0x100) {
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_BYTE);
		wb_push(wb, &n, 1);
		uint8_t byte = (uint8_t)v;
		wb_push(wb, &byte, sizeof(byte));
	} else if (v<0x10000) {
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_WORD);
		wb_push(wb, &n, 1);
		uint16_t word = (uint16_t)v;
		wb_push(wb, &word, sizeof(word));
	} else {
		uint8_t n = COMBINE_TYPE(type , TYPE_NUMBER_DWORD);
		wb_push(wb, &n, 1);
		uint32_t v32 = (uint32_t)v;
		wb_push(wb, &v32, sizeof(v32));
	}
}

static inline void
wb_real(struct write_block *wb, double v) {
	uint8_t n = COMBINE_TYPE(TYPE_NUMBER , TYPE_NUMBER_REAL);
	wb_push(wb, &n, 1);
	wb_push(wb, &v, sizeof(v));
}

static inline void
wb_pointer(struct write_block *wb, void *v) {
	int n = TYPE_USERDATA;
	wb_push(wb, &n, 1);
	wb_push(wb, &v, sizeof(v));
}

static inline void
wb_function(struct write_block *wb, thread::CallbackContext *v) {
    int n = TYPE_FUNCTION;
    wb_push(wb, &n, 1);
    wb_push(wb, &v, sizeof(v));
}

static inline void
wb_string(struct write_block *wb, const char *str, int len) {
	if (len < MAX_COOKIE) {
		int n = COMBINE_TYPE(TYPE_SHORT_STRING, len);
		wb_push(wb, &n, 1);
		if (len > 0) {
			wb_push(wb, str, len);
		}
	} else {
		int n;
		if (len < 0x10000) {
			n = COMBINE_TYPE(TYPE_LONG_STRING, 2);
			wb_push(wb, &n, 1);
			uint16_t x = (uint16_t) len;
			wb_push(wb, &x, 2);
		} else {
			n = COMBINE_TYPE(TYPE_LONG_STRING, 4);
			wb_push(wb, &n, 1);
			uint32_t x = (uint32_t) len;
			wb_push(wb, &x, 4);
		}
		wb_push(wb, str, len);
	}
}

static void _pack_one(lua_State *L, struct write_block *b, int index, int depth, std::list<thread::CallbackContext *> & callbackContext);

static int
wb_table_array(lua_State *L, struct write_block * wb, int index, int depth, std::list<thread::CallbackContext *> & callbackContext) {
	int array_size = (int)lua_objlen(L,index);
	if (array_size >= MAX_COOKIE-1) {
		int n = COMBINE_TYPE(TYPE_TABLE, MAX_COOKIE-1);
		wb_push(wb, &n, 1);
		wb_integer(wb, array_size);
	} else {
		int n = COMBINE_TYPE(TYPE_TABLE, array_size);
		wb_push(wb, &n, 1);
	}

	int i;
	for (i=1;i<=array_size;i++) {
		lua_rawgeti(L,index,i);
		_pack_one(L, wb, -1, depth,callbackContext);
		lua_pop(L,1);
	}

	return array_size;
}

static void
wb_table_hash(lua_State *L, struct write_block * wb, int index, int depth, int array_size, std::list<thread::CallbackContext *> & callbackContext) {
	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		if (lua_type(L,-2) == LUA_TNUMBER) {
			lua_Number k = lua_tonumber(L,-2);
			int32_t x = (int32_t)lua_tointeger(L,-2);
			if (k == (lua_Number)x && x>0 && x<=array_size) {
				lua_pop(L,1);
				continue;
			}
		}
		_pack_one(L,wb,-2,depth,callbackContext);
		_pack_one(L,wb,-1,depth,callbackContext);
		lua_pop(L, 1);
	}
	wb_nil(wb);
}

static void
wb_table(lua_State *L, struct write_block *wb, int index, int depth, std::list<thread::CallbackContext *> & callbackContext) {
	luaL_checkstack(L,LUA_MINSTACK,NULL);
	if (index < 0) {
		index = lua_gettop(L) + index + 1;
	}
	int array_size = wb_table_array(L, wb, index, depth,callbackContext);
	wb_table_hash(L, wb, index, depth, array_size,callbackContext);
}

#if LUA_VERSION_NUM < 503

static int
lua_isinteger(lua_State *L, int index) {
	int32_t x = (int32_t)lua_tointeger(L,index);
	lua_Number n = lua_tonumber(L,index);
	return ((lua_Number)x==n);
}

#endif

static void
_pack_one(lua_State *L, struct write_block *b, int index, int depth,std::list<thread::CallbackContext *> & callbackContexts) {
	if (depth > MAX_DEPTH) {
		wb_free(b);
		luaL_error(L, "serialize can't pack too depth table");
	}
	int type = lua_type(L,index);
	switch(type) {
    case LUA_TUSERDATA:{
            thread::CallbackContext** callbackContext = (thread::CallbackContext **)luaL_checkudata(L, index, LUA_CALLBACK_METATABLE_NAME);
            callbackContexts.push_back(*callbackContext);
            wb_function(b, *callbackContext);
        break;
        }
        break;
    case LUA_TFUNCTION:{
            thread::CallbackContext *callbackContext = new thread::CallbackContext();
            BusinessThreadID from_thread_identifier;
            BusinessThread::GetCurrentThreadIdentifier(&from_thread_identifier);
            callbackContext->fromThread = from_thread_identifier;
            callbackContexts.push_back(callbackContext);
            lua_pushvalue(L, index);
            pushStrongUserdataTable(L);
            lua_pushlightuserdata(L, callbackContext);
            lua_pushvalue(L,-3);
            lua_rawset(L, -3);
            lua_pop(L, 2);
            wb_function(b, callbackContext);
        }
        break;
	case LUA_TNIL:
		wb_nil(b);
		break;
	case LUA_TNUMBER: {
		if (lua_isinteger(L, index)) {
			lua_Integer x = lua_tointeger(L,index);
			wb_integer(b, x);
		} else {
			lua_Number n = lua_tonumber(L,index);
			wb_real(b,n);
		}
		break;
	}
	case LUA_TBOOLEAN: 
		wb_boolean(b, lua_toboolean(L,index));
		break;
	case LUA_TSTRING: {
		size_t sz = 0;
		const char *str = lua_tolstring(L,index,&sz);
		wb_string(b, str, (int)sz);
		break;
	}
	case LUA_TLIGHTUSERDATA:
		wb_pointer(b, lua_touserdata(L,index));
		break;
	case LUA_TTABLE:
		wb_table(L, b, index, depth+1,callbackContexts);
		break;
	default:
		wb_free(b);
		luaL_error(L, "Unsupport type %s to serialize", lua_typename(L, type));
	}
}

static void
_pack_from(lua_State *L, struct write_block *b, int count, std::list<thread::CallbackContext *> & callbackContext) {
	int from = lua_gettop(L) - count + 1;
	int i;
	for (i=from;i<=lua_gettop(L);i++) {
		_pack_one(L, b , i, 0, callbackContext);
	}
}

extern struct block *
seri_pack(lua_State *L ,std::list<thread::CallbackContext *> & callbackContext, int count) {
    if (count < 0) {
        count = lua_gettop(L);
    }
	struct write_block b;
	wb_init(&b, NULL);
	_pack_from(L,&b,count,callbackContext);
	struct block * ret = wb_close(&b);
	return ret;
}

/*
static int
_append(lua_State *L) {
	struct write_block b;
	wb_init(&b, lua_touserdata(L,1));
	_pack_from(L,&b,1);
	struct block * ret = wb_close(&b);
	lua_pushlightuserdata(L,ret);
	return 1;
}
*/

static inline void
invalid_stream_line(lua_State *L, struct read_block *rb, int line) {
	int len = rb->len;
	luaL_error(L, "Invalid serialize stream %d (line:%d)", len, line);
}

#define invalid_stream(L,rb) invalid_stream_line(L,rb,__LINE__)

static lua_Integer
get_integer(lua_State *L, struct read_block *rb, int cookie) {
	switch (cookie) {
	case TYPE_NUMBER_ZERO:
		return 0;
	case TYPE_NUMBER_BYTE: {
		uint8_t n;
		uint8_t * pn = (uint8_t *)rb_read(rb,&n,sizeof(n));
		if (pn == NULL)
			invalid_stream(L,rb);
		n = *pn;
		return n;
	}
	case TYPE_NUMBER_WORD: {
		uint16_t n;
		uint16_t * pn = (uint16_t *)rb_read(rb,&n,sizeof(n));
		if (pn == NULL)
			invalid_stream(L,rb);
		memcpy(&n, pn, sizeof(n));
		return n;
	}
	case TYPE_NUMBER_DWORD: {
		int32_t n;
		int32_t * pn = (int32_t *)rb_read(rb,&n,sizeof(n));
		if (pn == NULL)
			invalid_stream(L,rb);
		memcpy(&n, pn, sizeof(n));
		return n;
	}
	case TYPE_NUMBER_QWORD: {
		int64_t n;
		int64_t * pn = (int64_t *)rb_read(rb,&n,sizeof(n));
		if (pn == NULL)
			invalid_stream(L,rb);
		memcpy(&n, pn, sizeof(n));
		return n;
	}
	default:
		invalid_stream(L,rb);
		return 0;
	}
}

static double
get_real(lua_State *L, struct read_block *rb) {
	double n;
	double * pn = (double *)rb_read(rb,&n,sizeof(n));
	if (pn == NULL)
		invalid_stream(L,rb);
	memcpy(&n, pn, sizeof(n));
	return n;
}

static void *
_get_pointer(lua_State *L, struct read_block *rb) {
	void * userdata = 0;
	void ** v = (void **)rb_read(rb,&userdata,sizeof(userdata));
	if (v == NULL) {
		invalid_stream(L,rb);
	}
	return *v;
}

static void
_get_buffer(lua_State *L, struct read_block *rb, int len) {
	char tmp[len];
	char * p = (char *)rb_read(rb,tmp,len);
	lua_pushlstring(L,p,len);
}

static void _unpack_one(lua_State *L, struct read_block *rb);

static void
_unpack_table(lua_State *L, struct read_block *rb, int array_size) {
	if (array_size == MAX_COOKIE-1) {
		uint8_t type;
		uint8_t *t = (uint8_t *)rb_read(rb, &type, sizeof(type));
		if (t==NULL) {
			invalid_stream(L,rb);
		}
		type = *t;
		int cookie = type >> 3;
		if ((type & 7) != TYPE_NUMBER || cookie == TYPE_NUMBER_REAL) {
			invalid_stream(L,rb);
		}
		array_size = get_integer(L,rb,cookie);
	}
	luaL_checkstack(L,LUA_MINSTACK,NULL);
	lua_createtable(L,array_size,0);
	int i;
	for (i=1;i<=array_size;i++) {
		_unpack_one(L,rb);
		lua_rawseti(L,-2,i);
	}
	for (;;) {
		_unpack_one(L,rb);
		if (lua_isnil(L,-1)) {
			lua_pop(L,1);
			return;
		}
		_unpack_one(L,rb);
		lua_rawset(L,-3);
	}
}

static void
_push_value(lua_State *L, struct read_block *rb, int type, int cookie) {
	switch(type) {
	case TYPE_NIL:
		lua_pushnil(L);
		break;
    case TYPE_FUNCTION: {
            thread::CallbackContext *callbackContext =(thread::CallbackContext *)_get_pointer(L,rb);
            BusinessThreadID now_thread_identifier;
            BusinessThread::GetCurrentThreadIdentifier(&now_thread_identifier);
            if (now_thread_identifier == callbackContext->fromThread) {
                //同一线程切换,被传出去又传回来
                pushUserdataInStrongTable(L, callbackContext);
                if (callbackContext->toTheadList.size() == 0) {
                    delete callbackContext;
                }
            } else {
                pushUserdataInWeakTable(L,callbackContext);
                if (lua_isnil(L, -1)){
                    lua_pop(L, 1);
                } else {
                    return;
                }
                //如果weak表里面还没有，加进weak表
                size_t nbytes = sizeof(thread::CallbackContext *);
                thread::CallbackContext ** instanceUserdata = (thread::CallbackContext **)lua_newuserdata(L, nbytes);
                *instanceUserdata = callbackContext;
                // set the metatable
                luaL_getmetatable(L, LUA_CALLBACK_METATABLE_NAME);
                lua_setmetatable(L, -2);
                // give it a nice clean environment
                lua_newtable(L);
                lua_setfenv(L, -2);
                pushWeakUserdataTable(L);
                lua_pushlightuserdata(L, callbackContext);
                lua_pushvalue(L, -3);
                lua_rawset(L, -3);
                lua_pop(L, 1);// Pop off weak table
            }
        }
        break;
	case TYPE_BOOLEAN:
		lua_pushboolean(L,cookie);
		break;
	case TYPE_NUMBER:
		if (cookie == TYPE_NUMBER_REAL) {
			lua_pushnumber(L,get_real(L,rb));
		} else {
			lua_pushinteger(L, get_integer(L, rb, cookie));
		}
		break;
	case TYPE_USERDATA:
		lua_pushlightuserdata(L,_get_pointer(L,rb));
		break;
	case TYPE_SHORT_STRING:
		_get_buffer(L,rb,cookie);
		break;
	case TYPE_LONG_STRING: {
		uint32_t len;
		if (cookie == 2) {
			uint16_t *plen = (uint16_t *)rb_read(rb, &len, 2);
			if (plen == NULL) {
				invalid_stream(L,rb);
			}
			_get_buffer(L,rb,(int)*plen);
		} else {
			if (cookie != 4) {
				invalid_stream(L,rb);
			}
			uint32_t *plen = (uint32_t *)rb_read(rb, &len, 4);
			if (plen == NULL) {
				invalid_stream(L,rb);
			}
			_get_buffer(L,rb,(int)*plen);
		}
		break;
	}
	case TYPE_TABLE: {
		_unpack_table(L,rb,cookie);
		break;
	}
	}
}

static void
_unpack_one(lua_State *L, struct read_block *rb) {
	uint8_t type = 0;
	uint8_t *t = (uint8_t *)rb_read(rb, &type, 1);
	if (t==NULL) {
		invalid_stream(L, rb);
	}
	_push_value(L, rb, *t & 0x7, *t>>3);
}

extern int
seri_unpack(lua_State *L) {
	struct block * blk = (struct block *)lua_touserdata(L,-1);
	if (blk == NULL) {
		return luaL_error(L, "Need a block to unpack");
	}
	lua_settop(L,0);
	struct read_block rb;
	rb_init(&rb, blk);

	int i;
	for (i=0;;i++) {
		if (i%8==7) {
			luaL_checkstack(L,LUA_MINSTACK,NULL);
		}
		uint8_t type = 0;
		uint8_t *t = (uint8_t *)rb_read(&rb, &type, 1);
		if (t==NULL)
			break;
		_push_value(L, &rb, *t & 0x7, *t>>3);
	}

	rb_close(&rb);

	return lua_gettop(L);
}
