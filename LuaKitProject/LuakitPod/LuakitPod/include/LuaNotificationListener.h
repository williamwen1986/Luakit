#pragma once

#include "notification_observer.h"
#include "notification_registrar.h"
#include "notification_details.h"
#include "notification_service.h"
#include "notification_source.h"
#include "ref_counted.h"
#include "scoped_ptr.h"
#include "LuaNotificationListener.h"
#include "business_runtime.h"
#include <set>

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

