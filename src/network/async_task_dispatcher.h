#pragma once

#include <vector>
#include <list>
#include <map>

#include "base/memory/ref_counted.h"
#include "common/notification_observer.h"
#include "common/notification_registrar.h"
#include "common/business_client_thread.h"
#include "network/network_define.h"

namespace network {

class CurlConnection;

class AsyncTaskDispatcher : public base::RefCountedThreadSafe<AsyncTaskDispatcher, BusinessThread::DeleteOnIOThread>,
                            public content::NotificationObserver {
public:
    /**
     * @param fifo为true代表相同优先级的任务先进先出，否则后进先出
     */
    explicit AsyncTaskDispatcher(const std::string& name, bool fifo);
    virtual ~AsyncTaskDispatcher();

protected:
    // fix: dox 此方法可能触发上层逻辑把this析构，所以需要保护一下this的引用计数
    static void ResetConnection(const scoped_refptr<AsyncTaskDispatcher>& dispatcher, bool finish_doing);
    
    virtual ProtocolErrorCode AsyncProcess(CurlConnection* pConnection) ;

    void CreateFixedConnections(int count, bool auto_reset = true, const std::string& name = "") ;
    int GetIdleConnection() ;
    CurlConnection* GetFlaggedConnection() ;
    void ReleaseBusyConnection(CurlConnection* pConnection) ;
    
    void ScheduleTasksWithPriority() ;
    void ScheduleTasksWithPriority(const base::Closure& closure, int priority) ;

    typedef base::Callback<void(ProtocolErrorCode)> FinishCallback;
    void CreateSocketWatcher(CurlConnection* pConnection, const CompleteCallback& complete_callback);
  
    void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) override;
  
private:
    typedef std::list<scoped_refptr<NetWorkTask> > NETWORK_TASK_PRIORITY_LIST ;
    NETWORK_TASK_PRIORITY_LIST network_task_priority_list_ ;
    
    typedef std::map<int, CurlConnection*> MAP_CONNECTION_BUNDLE;
    MAP_CONNECTION_BUNDLE connection_pool_ ;
    
    std::string dispatcher_name_;
    bool fifo_queue_;
  
    std::unique_ptr<content::NotificationRegistrar> registrar_;
};
}
