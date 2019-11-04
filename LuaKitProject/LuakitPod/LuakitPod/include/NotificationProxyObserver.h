#pragma once
extern "C" {
#include "lua.h"
}
#include "notification_observer.h"
#include "notification_registrar.h"
#include "notification_details.h"
#include "notification_service.h"
#include "notification_source.h"
#include "lua_notify.h"
#include "oc_helpers.h"

@protocol NotificationProxyObserverDelegate
@required
- (void)onNotification:(int)type data:(id)data;
@end

class NotificationProxyObserver : public content::NotificationObserver {
public:
    NotificationProxyObserver(id<NotificationProxyObserverDelegate> ob) : delegate_(ob) { }
    NotificationProxyObserver(id<NotificationProxyObserverDelegate> ob, int type) : delegate_(ob) {
        AddObserver(type, content::NotificationService::AllSources());
    }
    
    virtual ~NotificationProxyObserver() {
    }
    
    void AddObserver(int type) {
        registrar_.Add(this, type, content::NotificationService::AllSources());
    }
    
    void AddObserver(int type, const content::NotificationSource& source) {
        registrar_.Add(this, type, source);
    }
    
    void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) {
        content::Source<Notification::sourceType> s(source);
        Notification::sourceType sourceType = (*s.ptr());
        switch (sourceType) {
            case Notification::LUA:
            {
                content::Details<lua_State> d(details);
                if (delegate_) {
                    id data = oc_copyToObjc(d.ptr(),-1);
                    [delegate_ onNotification:type data:data];
                }
            }
                break;
            default:
            {
                if (delegate_) {
                    content::Details<void> content(details);
                    void * d = content.ptr();
                    id data = (__bridge id)d;
                    [delegate_ onNotification:type data:data];
                }
            }
                break;
        }
        
    }
private:
    __weak id<NotificationProxyObserverDelegate> delegate_;
    content::NotificationRegistrar registrar_;
};

