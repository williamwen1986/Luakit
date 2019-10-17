extern class LuakitExtension TheAsyncSocketExtension;
extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension TheTimerExtension;
extern class LuakitExtension TheNetworkExtension;
extern class LuakitExtension* ExtensionsList [] =
{
    &TheAsyncSocketExtension,
    &TheThreadExtension,
    &TheTimerExtension,
    &TheNetworkExtension,
	0
};
