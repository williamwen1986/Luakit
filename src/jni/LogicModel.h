#ifndef LOGICMODEL_H_
#define LOGICMODEL_H_
#include "notification_listener_impl.h"
#include "JNIConversionDecl.h"

#define DECLARE_MODEL_BEGIN(subnamespace, signature) \
    namespace subnamespace { \
        DEFINE_SIGNATURE(signature)

#define DECLARE_MODEL_CONVERSION(model) \
            jobject ConvertToJava(JNIEnv* env, const model& native); \
            void ConvertToNative(JNIEnv* env, jobject obj, model& native);

#define DECLARE_MODEL_END() \
    };

#define DECLARE_FUNCTION(name, signature) \
    static const JNIModel::Function name = { #name, signature };

namespace LogicModel {
    DECLARE_MODEL_BEGIN(NotificationListener, "com/common/luakit/LuaNotificationListener")
        DECLARE_MODEL_CONVERSION(scoped_refptr<NotificationListenerImpl>)
        DECLARE_FUNCTION(onObserve, "(ILjava/lang/Object;)V")
    DECLARE_MODEL_END()
}
#endif /* LOGICMODEL_H_ */
