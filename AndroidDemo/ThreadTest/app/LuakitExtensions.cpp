#include "jniLinkedModules.h" // I'm not proud of this hack. (I have fight much time to force the link-editor to link-edit correctly jni).
extern class LuakitExtension TheThreadExtension;
extern class LuakitExtension* ExtensionsList [] =
{
    &TheThreadExtension,
	0
};
