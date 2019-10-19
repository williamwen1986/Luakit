extern class LuakitExtension TheCjsonExtension;
extern class LuakitExtension TheFileExtension;
extern class LuakitExtension TheAsyncSocketExtension;
extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension TheTimerExtension;
extern class LuakitExtension TheLuaLanguageExtension;
extern class LuakitExtension TheDebugExtension;
extern class LuakitExtension TheHTTPExtension;
extern class LuakitExtension TheSQLiteExtension;
extern class LuakitExtension TheNotifyExtension;

extern class LuakitExtension* ExtensionsList [] =
{
    &TheSQLiteExtension,
    &TheThreadExtension,
    &TheTimerExtension,
    &TheLuaLanguageExtension,
    &TheCjsonExtension,
    &TheAsyncSocketExtension,
    &TheDebugExtension,
    &TheFileExtension,
    &TheHTTPExtension,
    &TheNotifyExtension,
	0
};
