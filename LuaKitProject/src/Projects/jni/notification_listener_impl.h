#ifndef ANDROID_NOTIFICATION_LISTENER_IMPL_H
#define ANDROID_NOTIFICATION_LISTENER_IMPL_H
#include "common/notification_observer.h"
#include "common/notification_registrar.h"
#include "common/notification_details.h"
#include "common/notification_service.h"
#include "common/notification_source.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "JavaRefCountedWrapper.h"
#include "java_weak_ref.h"

class NotificationListenerImpl : public content::NotificationObserver,
                                 public base::RefCountedThreadSafe<NotificationListenerImpl>{
public:
    NotificationListenerImpl(const jobject& jlistener);
    void AddObserver(int type);
    void RemoveObserver(int type);

private:
    void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details);

private:
    scoped_ptr<java_weak_ref> jlistener_;
    content::NotificationRegistrar registrar_;
};


#endif //ANDROID_NOTIFICATION_LISTENER_IMPL_H
