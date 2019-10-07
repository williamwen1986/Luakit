#include <string>
#include <algorithm>
#include "LuakitLoader.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "lua_helpers.h"
extern "C"
{
    int luakit_loader(lua_State *L)
    {
        static const std::string BYTECODE_FILE_EXT    = ".luac";
        static const std::string NOT_BYTECODE_FILE_EXT = ".lua";

        std::string filename(luaL_checkstring(L, 1));
        size_t pos = filename.rfind(BYTECODE_FILE_EXT);
        if (pos != std::string::npos && pos == filename.length() - BYTECODE_FILE_EXT.length())
            filename = filename.substr(0, pos);
        else
        {
            pos = filename.rfind(NOT_BYTECODE_FILE_EXT);
            if (pos != std::string::npos && pos == filename.length() - NOT_BYTECODE_FILE_EXT.length())
                filename = filename.substr(0, pos);
        }

        pos = filename.find_first_of(".");
        while (pos != std::string::npos)
        {
            filename.replace(pos, 1, "/");
            pos = filename.find_first_of(".");
        }

        // search file in package.path
        std::string chunk;
        std::string chunkName;

        lua_getglobal(L, "package");
        lua_getfield(L, -1, "path");
        std::string searchpath(lua_tostring(L, -1));
        lua_pop(L, 1);
        size_t begin = 0;
        size_t next = searchpath.find_first_of(";", 0);

        do
        {
            if (next == std::string::npos)
                next = searchpath.length();
            std::string prefix = searchpath.substr(begin, next-begin);
            if (prefix[0] == '.' && prefix[1] == '/')
                prefix = prefix.substr(2);

            pos = prefix.rfind(BYTECODE_FILE_EXT);
            if (pos != std::string::npos && pos == prefix.length() - BYTECODE_FILE_EXT.length())
            {
                prefix = prefix.substr(0, pos);
            }
            else
            {
                pos = prefix.rfind(NOT_BYTECODE_FILE_EXT);
                if (pos != std::string::npos && pos == prefix.length() - NOT_BYTECODE_FILE_EXT.length())
                    prefix = prefix.substr(0, pos);
            }
            pos = prefix.find_first_of("?", 0);
            while (pos != std::string::npos)
            {
                prefix.replace(pos, 1, filename);
                pos = prefix.find_first_of("?", pos + filename.length() + 1);
            }
            chunkName = prefix + BYTECODE_FILE_EXT;
            base::FilePath fpath = base::FilePath(chunkName);
            if (base::PathExists(fpath) && !base::DirectoryExists(fpath))
            {
                base::ReadFileToString(fpath, &chunk);
                break;
            }
            else
            {
                chunkName = prefix + NOT_BYTECODE_FILE_EXT;
                fpath = base::FilePath(chunkName);
                if (base::PathExists(fpath) && !base::DirectoryExists(fpath))
                {
                    base::ReadFileToString(fpath, &chunk);
                    break;
                }
                else
                {
                    chunkName = prefix;
                    fpath = base::FilePath(chunkName);
                    if (base::PathExists(fpath) && !base::DirectoryExists(fpath))
                    {
                        base::ReadFileToString(fpath, &chunk);
                        break;
                    }
                }
            }

            begin = next + 1;
            next = searchpath.find_first_of(";", begin);
        } while (begin < searchpath.length());
        if (chunk.length() > 0)
        {
            luaLoadBuffer(L,chunk.c_str(),(int)chunk.length(),chunkName.c_str());
        }
        else
        {
            LOG(ERROR) << "can not get file data of"<<chunkName;
            return 0;
        }

        return 1;
    }
}
