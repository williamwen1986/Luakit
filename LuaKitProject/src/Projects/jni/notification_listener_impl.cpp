#include "notification_listener_impl.h"
#include "JavaRefCountedWrapper.h"
#include "JNIConversionDecl.h"
#include "JNIConversionImpl.h"
#include "JniEnvWrapper.h"
#include "JniLuaConvertor.h"
#include "LogicModel.h"
#include "lua_notify.h"
#include "common/notification_service.h"
#include "common/notification_details.h"
#include "common/notification_source.h"

NotificationListenerImpl::NotificationListenerImpl(const jobject& jlistener) :
        jlistener_(new java_weak_ref(jlistener)){

}

void NotificationListenerImpl::AddObserver(int type) {
    LOG(INFO) << "native add notification observer. type = " << type;

    registrar_.Add(this, type, content::NotificationService::AllSources());
}

void NotificationListenerImpl::RemoveObserver(int type) {
    LOG(INFO) << "native remove notification observer. type = " << type;
    registrar_.Remove(this, type, content::NotificationService::AllSources());
}

void NotificationListenerImpl::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) {
    JniEnvWrapper env;
    LOG(WARNING) << "NotificationListenerImpl after call java method";
    content::Source<Notification::sourceType> s(source);
    Notification::sourceType sourceType = (*s.ptr());
    switch (sourceType) {
        case Notification::LUA:
        {
        	content::Details<lua_State> d(details);
        	jobject data = object_copyToJava(d.ptr(), *env, -1);
        	env->PushLocalFrame(0);
		    env.CallVoidMethod(
		            jlistener_->obj(),
		            LogicModel::NotificationListener::classSig,
		            LogicModel::NotificationListener::onObserve.name,
		            LogicModel::NotificationListener::onObserve.sig,
		            type,
		            data
		            );
		    env->PopLocalFrame(NULL);
        }
            break;
        default:
        {
            content::Details<void> content(details);
    		void * d = content.ptr();
    		jobject data = (jobject)d;
    		env->PushLocalFrame(0);
		    env.CallVoidMethod(
		            jlistener_->obj(),
		            LogicModel::NotificationListener::classSig,
		            LogicModel::NotificationListener::onObserve.name,
		            LogicModel::NotificationListener::onObserve.sig,
		            type,
		            data
		            );
		    env->PopLocalFrame(NULL);
        }
            break;
    }

}
