#pragma once

#include "notification/notification_observer.h"
#include "notification/notification_registrar.h"
#include "notification/notification_details.h"
#include "notification/notification_service.h"
#include "notification/notification_source.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "LuaNotificationListener.h"
#include "common/business_runtime.h"

class LuaNotificationListener : public content::NotificationObserver,
public base::RefCountedThreadSafe<LuaNotificationListener>{
public:
    LuaNotificationListener();
    ~LuaNotificationListener();
    void AddObserver(int type);
    void RemoveObserver(int type);

private:
    void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details);
public :
    BusinessThreadID threadId;
private:
    content::NotificationRegistrar registrar_;
    
};

