#include "LuaNotificationListener.h"
#include "notification_details.h"
#include "notification_source.h"

extern void notification_call_to_lua(LuaNotificationListener * l, int type, const content::NotificationSource& source, const content::NotificationDetails& details);

