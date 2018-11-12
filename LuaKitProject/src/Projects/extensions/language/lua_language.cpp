extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "lua_helpers.h"
#include "lua_language.h"
#include <string>
#include "languageUtil.h"

enum LanguageType {
    ENGLISH,
    CHINESE,
    CHINESE_TW,
    JAPANESE,
    FRENCH,
    ITALIAN,
    GERMAN,
    SPANISH,
    DUTCH,
    RUSSIAN,
    HUNGARIAN,
    PORTUGUESE,
    KOREAN,
    ARABIC,
    NORWEGIAN,
    POLISH,
    TURKISH,
    UKRAINIAN,
    ROMANIAN,
    BULGARIAN,
    BELARUSIAN,
};

LanguageType getLanguageTypeByISO2(const char* code)
{
    // this function is used by all platforms to get system language
    // except windows: cocos/platform/win32/CCApplication-win32.cpp
    LanguageType ret = LanguageType::ENGLISH;
    std::string language = code;
    if (strncmp(code, "zh", 2) == 0)
    {
        if (language=="zh-HK"||language=="zh-TW"||language=="zh-Hant-CN"||language=="zh-Hant") {
            ret = LanguageType::CHINESE_TW;
        } else {
            ret = LanguageType::CHINESE;
        }
    }
    else if (strncmp(code, "ja", 2) == 0)
    {
        ret = LanguageType::JAPANESE;
    }
    else if (strncmp(code, "fr", 2) == 0)
    {
        ret = LanguageType::FRENCH;
    }
    else if (strncmp(code, "it", 2) == 0)
    {
        ret = LanguageType::ITALIAN;
    }
    else if (strncmp(code, "de", 2) == 0)
    {
        ret = LanguageType::GERMAN;
    }
    else if (strncmp(code, "es", 2) == 0)
    {
        ret = LanguageType::SPANISH;
    }
    else if (strncmp(code, "nl", 2) == 0)
    {
        ret = LanguageType::DUTCH;
    }
    else if (strncmp(code, "ru", 2) == 0)
    {
        ret = LanguageType::RUSSIAN;
    }
    else if (strncmp(code, "hu", 2) == 0)
    {
        ret = LanguageType::HUNGARIAN;
    }
    else if (strncmp(code, "pt", 2) == 0)
    {
        ret = LanguageType::PORTUGUESE;
    }
    else if (strncmp(code, "ko", 2) == 0)
    {
        ret = LanguageType::KOREAN;
    }
    else if (strncmp(code, "ar", 2) == 0)
    {
        ret = LanguageType::ARABIC;
    }
    else if (strncmp(code, "nb", 2) == 0)
    {
        ret = LanguageType::NORWEGIAN;
    }
    else if (strncmp(code, "pl", 2) == 0)
    {
        ret = LanguageType::POLISH;
    }
    else if (strncmp(code, "tr", 2) == 0)
    {
        ret = LanguageType::TURKISH;
    }
    else if (strncmp(code, "uk", 2) == 0)
    {
        ret = LanguageType::UKRAINIAN;
    }
    else if (strncmp(code, "ro", 2) == 0)
    {
        ret = LanguageType::ROMANIAN;
    }
    else if (strncmp(code, "bg", 2) == 0)
    {
        ret = LanguageType::BULGARIAN;
    }
    else if (strncmp(code, "be", 2) == 0)
    {
        ret = LanguageType::BELARUSIAN;
    }
    return ret;
}

static int getLanguageType(lua_State *L)
{
    BEGIN_STACK_MODIFY(L)
    lua_pushinteger(L, getLanguageTypeByISO2(languageType()));
    END_STACK_MODIFY(L, 1)
    return 1;
}

static const struct luaL_Reg metaFunctions[] = {
    {NULL, NULL}
};

static const struct luaL_Reg functions[] = {
    {"getLanguageType", getLanguageType},
    {NULL, NULL}
};

extern int luaopen_language(lua_State* L)
{
    BEGIN_STACK_MODIFY(L);
    luaL_newmetatable(L, LUA_LANGUAGE_METATABLE_NAME);
    luaL_register(L, NULL, metaFunctions);
    luaL_register(L, LUA_LANGUAGE_METATABLE_NAME, functions);
    END_STACK_MODIFY(L, 0)
    return 0;
}

