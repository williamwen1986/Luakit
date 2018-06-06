#ifdef __cplusplus
extern "C" {
#endif
#include "lauxlib.h"
#include "lapi.h"
#ifdef __cplusplus
}
#endif
#include <mutex>
#include "JniEnvWrapper.h"
#include "JniLuaConvertor.h"
#include "JNIModel.h"
#include "JniClassMember.h"
#include "JniCallbackHelper.h"
static jclass BooleanClass;
static jclass IntegerClass;
static jclass ByteClass;
static jclass LongClass;
static jclass FloatClass;
static jclass DoubleClass;
static jclass HashmapClass;
static jclass StringClass;

static bool hasInitClass = false;
static std::recursive_mutex  globle_lock;

static int
lua_isinteger(lua_State *L, int index) {
    int32_t x = (int32_t)lua_tointeger(L,index);
    lua_Number n = lua_tonumber(L,index);
    return ((lua_Number)x==n);
}

static bool isJavaArray(JNIEnv *env , jobject o)
{
    jclass clazz = env->GetObjectClass(o);
    jmethodID mid = env->GetMethodID(clazz, "getClass", "()Ljava/lang/Class;");
    jobject clsObj = env->CallObjectMethod(o, mid);
    clazz = env->GetObjectClass(clsObj);
    mid = env->GetMethodID(clazz, "isArray", "()Z");
    jboolean booleanObj = (jboolean)env->CallBooleanMethod(clsObj, mid);
    return booleanObj == JNI_TRUE;
}

static void initClass (JNIEnv *env){
    std::lock_guard<std::recursive_mutex> guard(globle_lock);
    if (hasInitClass == false)
    {
        BooleanClass = env->FindClass(JNIModel::Boolean::classSig);
        BooleanClass = (jclass)env->NewGlobalRef(BooleanClass);
        IntegerClass = env->FindClass(JNIModel::Integer::classSig);
        IntegerClass = (jclass)env->NewGlobalRef(IntegerClass);
        ByteClass = env->FindClass(JNIModel::Byte::classSig);
        ByteClass = (jclass)env->NewGlobalRef(ByteClass);
        LongClass = env->FindClass(JNIModel::Long::classSig);
        LongClass = (jclass)env->NewGlobalRef(LongClass);
        FloatClass = env->FindClass(JNIModel::Float::classSig);
        FloatClass = (jclass)env->NewGlobalRef(FloatClass);
        DoubleClass = env->FindClass(JNIModel::Double::classSig);
        DoubleClass = (jclass)env->NewGlobalRef(DoubleClass);
        StringClass = env->FindClass(JNIModel::String::classSig);
        StringClass = (jclass)env->NewGlobalRef(StringClass);
        HashmapClass = env->FindClass(JNIModel::HashMap::classSig);
        HashmapClass = (jclass)env->NewGlobalRef(HashmapClass);
        hasInitClass = true;
    }
}

static void pushOneObject(lua_State *L, JNIEnv *env, jobject o)
{
    if(env->IsSameObject(o, NULL)){
        lua_pushnil(L);
    } else if(env->IsInstanceOf(o,BooleanClass)){
        bool b = JNIModel::Boolean::ConvertToNative(env,o);
        lua_pushboolean(L, b);
    } else if (env->IsInstanceOf(o,IntegerClass)){
        int i = JNIModel::Integer::ConvertToNative(env,o);
        lua_pushinteger(L, i);
    } else if (env->IsInstanceOf(o,LongClass)) {
        long l = JNIModel::Long::ConvertToNative(env,o);
        lua_pushinteger(L, l);
    } else if (env->IsInstanceOf(o,FloatClass)){
        float f = JNIModel::Float::ConvertToNative(env,o);
        lua_pushnumber(L, f);
    } else if (env->IsInstanceOf(o,DoubleClass)) {
        double d = JNIModel::Double::ConvertToNative(env,o);
        lua_pushnumber(L, d);
    } else if (env->IsInstanceOf(o,StringClass)){
        std::string s = JNIModel::String::ConvertToNative(env,o);
        lua_pushlstring(L, s.c_str(), s.size());
    } else if (env->IsInstanceOf(o,HashmapClass)) {
        lua_newtable(L);
        JniEnvWrapper envw(env);
        env->PushLocalFrame(0);
        jobject entryset = envw.CallObjectMethod(o, JNIModel::HashMap::classSig, JNIModel::HashMap::entrySet.name, JNIModel::HashMap::entrySet.sig);
        jobject iterator = envw.CallObjectMethod(entryset, JNIModel::Set::classSig, JNIModel::Set::iterator.name, JNIModel::Set::iterator.sig);
        JniClassMember* jcm = JniClassMember::GetInstance();
        while (true) {
            jboolean hasNext = envw.CallBooleanMethod(iterator, JNIModel::Iterator::classSig, JNIModel::Iterator::hasNext.name, JNIModel::Iterator::hasNext.sig);
            if (hasNext == JNI_FALSE) {
                break;
            }
            jobject entry = envw.CallObjectMethod(iterator, JNIModel::Iterator::classSig, JNIModel::Iterator::next.name, JNIModel::Iterator::next.sig);
            jobject key = env->CallObjectMethod(entry, jcm->GetMethod(env, jcm->GetClass(env, JNIModel::Entry::classSig), JNIModel::Entry::classSig, JNIModel::Entry::getKey.name, JNIModel::Entry::getKey.sig));
            jobject val = env->CallObjectMethod(entry, jcm->GetMethod(env, jcm->GetClass(env, JNIModel::Entry::classSig), JNIModel::Entry::classSig, JNIModel::Entry::getValue.name, JNIModel::Entry::getValue.sig));
            pushOneObject(L, env, key);
            pushOneObject(L, env, val);
            lua_rawset(L, -3);
            env->DeleteLocalRef(entry);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(val);
        }
        env->PopLocalFrame(NULL);
    } else if (isJavaArray(env,o)){
        int length = env->GetArrayLength((jobjectArray)o);
        if (length > 0)
        {
            jobject val = (jobject)env->GetObjectArrayElement((jobjectArray)o, 0);
            if (env->IsInstanceOf(val,ByteClass))
            {
                char * cPtr = new char [length];
                for (int i = 0; i < length; i++) {
                    env->PushLocalFrame(0);
                    jobject val = (jobject)env->GetObjectArrayElement((jobjectArray)o, i);
                    cPtr[i] = JNIModel::Byte::ConvertToNative(env,val);
                    env->PopLocalFrame(NULL);
                }
                lua_pushlstring(L, cPtr, length);
                delete []cPtr;
            } else {
                lua_newtable(L);
                for (int i = 0; i < length; i++) {
                    env->PushLocalFrame(0);
                    jobject val = (jobject)env->GetObjectArrayElement((jobjectArray)o, i);
                    lua_pushinteger(L,i+1);
                    pushOneObject(L,env,val);
                    lua_rawset(L, -3);
                    env->PopLocalFrame(NULL);
                }
            }
        } else {
            lua_newtable(L);
        }
    } else {
        jclass clazz = env->GetObjectClass(o);
        jmethodID methodid = env->GetMethodID(clazz, "onResult", "(Ljava/lang/Object;)V");
        if (methodid != NULL)
        {
            push_jni_block(L,o);
        } else {
            luaL_error(L, "pushOneObject error type is illigel");
        }
    }
}

extern void object_fromjava(lua_State *L, JNIEnv *env, jobject o)
{
    initClass(env);
    pushOneObject(L,env,o);

}

static jobject getTableObject(lua_State *L, JNIEnv *env, int stackIndex) {
    bool dictionary = false;
    lua_pushvalue(L, stackIndex);
    lua_pushnil(L);  /* first key */
    while (!dictionary && lua_next(L, -2)) {
        if (lua_type(L, -2) != LUA_TNUMBER) {
            dictionary = true;
            lua_pop(L, 2); // pop key and value off the stack
        }
        else {
            lua_pop(L, 1);
        }
    }
    jobject instance;
    if (dictionary) {
        JniEnvWrapper envw(env);
        jclass clazz = env->FindClass(JNIModel::HashMap::classSig);
        jmethodID methodid = env->GetMethodID(clazz, JNIModel::HashMap::init.name, JNIModel::HashMap::init.sig);
        instance = env->NewObject(clazz, methodid);
        lua_pushnil(L);  /* first key */
        while (lua_next(L, -2)) {
            jobject key = object_copyToJava(L, env, -2);
            jobject val = object_copyToJava(L, env, -1);
            jobject ret = envw.CallObjectMethod(instance, JNIModel::HashMap::classSig, JNIModel::HashMap::put.name, JNIModel::HashMap::put.sig, key, val);
            lua_pop(L, 1); // Pop off the value
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(val);
            env->DeleteLocalRef(ret);
        } 
    } else {
        JniEnvWrapper envw(env);
        int length = (int)lua_objlen(L,-1);
        if (length == 0) {
            return NULL;
        }
        int index = 0;
        instance = envw.NewObjectArray("java/lang/Object", length, NULL);

        lua_pushnil(L);  /* first key */
        while (lua_next(L, -2)) {
            int index = lua_tonumber(L, -2) - 1;
            jobject element = object_copyToJava(L, env, -1);
            env->SetObjectArrayElement((jobjectArray)instance, index, element);
            env->DeleteLocalRef(element);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    return instance;
}

extern jobject object_copyToJava(lua_State *L, JNIEnv *env, int stackIndex)
{
    int type = lua_type(L,stackIndex);
    switch(type) {
        case LUA_TNIL:{
            return NULL;
        }
        break;
        case LUA_TBOOLEAN:{
            int ret = lua_toboolean(L, stackIndex);
            return JNIModel::Boolean::ConvertToJava(env,ret);
        }
        break;
        case LUA_TNUMBER:{
            if (lua_isinteger(L, stackIndex)) {
                lua_Integer ret = lua_tointeger(L,stackIndex);
                return JNIModel::Integer::ConvertToJava(env,ret);
            } else {
                lua_Number ret = lua_tonumber(L,stackIndex);
                return JNIModel::Double::ConvertToJava(env,ret);
            }
        }
        break;
        case LUA_TSTRING:{
            size_t size;
            const char * ret = lua_tolstring(L, stackIndex, &size);
            std::string s = std::string(ret, size);
            return JNIModel::String::ConvertToJava(env, s);
        }
        break;
        case LUA_TTABLE:{
            return getTableObject(L, env ,stackIndex);
        }
        break;
        default:
            luaL_error(L, "Unsupport type %s to getOneObject", lua_typename(L, type));
    }
    return NULL;
}