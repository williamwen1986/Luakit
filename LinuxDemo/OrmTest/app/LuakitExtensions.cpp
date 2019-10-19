extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension TheTimerExtension;
extern class LuakitExtension TheSQLiteExtension;
extern class LuakitExtension* ExtensionsList [] =
{
    &TheSQLiteExtension,
    &TheThreadExtension,
    &TheTimerExtension,
	0
};
