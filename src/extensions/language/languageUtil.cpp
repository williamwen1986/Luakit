#include "languageUtil.h"
#include "base/android/build_info.h"


const char * languageType()
{
	base::android::BuildInfo * info = base::android::BuildInfo::GetInstance();
	return info->language();
}