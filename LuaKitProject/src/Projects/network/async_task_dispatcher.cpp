#include "network/async_task_dispatcher.h"
#include "common/common/notification_service.h"
#include "common/business_client_thread.h"
#include "network/socket_watcher.h"
#include "network/curl_connection.h"
#include "common/common/network_monitor.h"
namespace network {
    
AsyncTaskDispatcher::AsyncTaskDispatcher(const std::string& name, bool fifo) :
    dispatcher_name_(name),
    fifo_queue_(fifo) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  registrar_.reset(new content::NotificationRegistrar);
  registrar_->Add(this, NOTIFICATION_NETWORK_CHANGED, content::NotificationService::AllSources());
}

AsyncTaskDispatcher::~AsyncTaskDispatcher() {
    for(auto& it : connection_pool_) {
        if(it.second != NULL) {
            delete it.second;
            it.second = NULL;
        }
    }
    connection_pool_.clear();
    if(network_task_priority_list_.size() > 0) {
        LOG(ERROR) << "悲剧了！dispatcher析构了，还有task没有跑完！这些东西不会有回调了！dispatcher name: " << dispatcher_name_ << " task list size: " << network_task_priority_list_.size();
    }
}

void AsyncTaskDispatcher::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) {
  auto thiz = make_scoped_refptr(this);
  if (type == NOTIFICATION_NETWORK_CHANGED) {
    ResetConnection(thiz, true);
  }
}

void AsyncTaskDispatcher::ResetConnection(const scoped_refptr<AsyncTaskDispatcher>& dispatcher, bool finish_doing) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  
  for(auto& value : dispatcher->connection_pool_) {
    CurlConnection* conn = value.second;
    if(conn != NULL) {
      if(finish_doing &&
         conn->ApplyStatus() == CurlConnection::APPLYSTATUS_BUSY) {
          LOG(WARNING) << conn->Name() << " 强制关闭正在进行的连接(#" << value.first << ")，触发失败回调逻辑。Dispatcher: " << dispatcher->dispatcher_name_;
          conn->ForceFinish(true);
      }
      conn->SetForceNewConnection(true);
    }
  }
}

ProtocolErrorCode AsyncTaskDispatcher::AsyncProcess(CurlConnection* pConnection) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    DCHECK(pConnection != NULL);
    int ret = pConnection->Perform();
    if(ret != CURLM_OK) {
        LOG(ERROR) << "AsyncTaskDispatcher AsyncProcess connection perform failed! curl multi error code: " << ret;
        pConnection->ForceFinish(false);
        return PEC_UNKNOW_ERROR;
    }
    ret = pConnection->CurlCode();
    LOG(INFO) << "AsyncTaskDispatcher AsyncProcess CurlCode: " << ret;
    if(ret != CURLE_OK) {
        LOG(ERROR) << "AsyncTaskDispatcher AsyncProcess failed! curl error code: " << ret;
        pConnection->ForceFinish(false);
        return PEC_UNKNOW_ERROR;
    }
    return PEC_OK;
}

void AsyncTaskDispatcher::CreateFixedConnections(int count, bool auto_reset, const std::string& name) {
    DCHECK(connection_pool_.empty());
    connection_pool_.clear() ;
    for (int i = 0; i < count; ++i) {
        CurlConnection* pConnection = new CurlConnection(i, name, auto_reset, this) ;
        connection_pool_.insert(std::make_pair(i, pConnection)) ;
    }
}

int AsyncTaskDispatcher::GetIdleConnection() {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    CurlConnection* pConnection = NULL ;
    
    for (MAP_CONNECTION_BUNDLE::iterator it = connection_pool_.begin();
         connection_pool_.end() != it;
         ++it) {
        pConnection = it->second;
        if (pConnection && CurlConnection::APPLYSTATUS_IDLE == pConnection->ApplyStatus()) {
            pConnection->SetStatus(CurlConnection::APPLYSTATUS_FLAGGED);
            return it->first ;
        }
    }
    
    return -1 ;
}

CurlConnection* AsyncTaskDispatcher::GetFlaggedConnection() {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    MAP_CONNECTION_BUNDLE::iterator it = connection_pool_.begin() ;
    for (; connection_pool_.end() != it; ++it) {
        CurlConnection* pConnection = it->second ;
        if (pConnection && CurlConnection::APPLYSTATUS_FLAGGED == pConnection->ApplyStatus())
        {
            pConnection->SetStatus(CurlConnection::APPLYSTATUS_BUSY);
            return pConnection ;
        }
    }
    NOTREACHED() << "逻辑错误了!";
    return NULL ;
}

void AsyncTaskDispatcher::ReleaseBusyConnection(CurlConnection* pConnection) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    if (NULL == pConnection) {
        return ;
    }
    pConnection->Clean();
    pConnection->SetStatus(CurlConnection::APPLYSTATUS_IDLE);
    ScheduleTasksWithPriority() ;
}


void AsyncTaskDispatcher::ScheduleTasksWithPriority() {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    if (network_task_priority_list_.empty()) {
        return;
    }
    
    NETWORK_TASK_PRIORITY_LIST::iterator it = network_task_priority_list_.begin() ;
  
    int nIndex = GetIdleConnection() ;
    if (-1 == nIndex) {
        return ;
    }
    BusinessThread::PostTask(BusinessThread::IO, FROM_HERE, (*it)->closure_) ;
    network_task_priority_list_.erase(it) ;
    LOG(WARNING) << dispatcher_name_ << " 连接池连接空闲，剩余任务数: " << network_task_priority_list_.size();
    return;
}

void AsyncTaskDispatcher::ScheduleTasksWithPriority(const base::Closure& f, int nPriority) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    int nIndex = GetIdleConnection() ;
    if (-1 == nIndex) {
        LOG(WARNING) << dispatcher_name_ << " 连接池暂时没有空闲连接，当前堆积任务数: " << network_task_priority_list_.size();
        NETWORK_TASK_PRIORITY_LIST::iterator it = network_task_priority_list_.begin() ;
        for (; network_task_priority_list_.end() != it; ++it) {
            if (fifo_queue_) {
                if (nPriority <= (*it)->priority_) continue ;
            } else {
                if (nPriority < (*it)->priority_) continue ;
            }
            network_task_priority_list_.insert(it, scoped_refptr<NetWorkTask>(new NetWorkTask(nPriority, f))) ;
            return;
        }
        network_task_priority_list_.push_back( scoped_refptr<NetWorkTask>(new NetWorkTask(nPriority, f))) ;
        return;
    }
    BusinessThread::PostTask(BusinessThread::IO, FROM_HERE, f) ;
}

void AsyncTaskDispatcher::CreateSocketWatcher(CurlConnection* pConnection, const CompleteCallback& complete_callback) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  pConnection->CreateSocketWatcher(complete_callback);
}

} // end of namespace network
