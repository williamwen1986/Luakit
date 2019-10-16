#include "jniLinkedModules.h" // I'm not proud of this hack. (I have fight much time to force the link-editor to link-edit correctly jni).

extern class LuakitExtension TheCjsonExtension;
extern class LuakitExtension TheFileExtension;
extern class LuakitExtension TheAsyncSocketExtension;
extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension TheTimerExtension;
extern class LuakitExtension TheLuaLanguageExtension;
extern class LuakitExtension TheDebugExtension;
extern class LuakitExtension TheHTTPExtension;

extern class LuakitExtension* ExtensionsList [] =
{
    &TheThreadExtension,
    &TheTimerExtension,
    &TheLuaLanguageExtension,
    &TheCjsonExtension,
    &TheAsyncSocketExtension,
    &TheDebugExtension,
    &TheFileExtension,
    &TheHTTPExtension,
	0
};
