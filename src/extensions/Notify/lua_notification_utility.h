#include "LuaNotificationListener.h"
#include "common/notification_details.h"
#include "common/notification_source.h"

extern void notification_call_to_lua(LuaNotificationListener * l, int type, const content::NotificationSource& source, const content::NotificationDetails& details);

