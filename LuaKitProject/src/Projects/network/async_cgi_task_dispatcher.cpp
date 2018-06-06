
#include "base/strings/string_number_conversions.h"
#include "base/guid.h"
#include "network/async_cgi_task_dispatcher.h"
#include "network/curl_connection.h"

namespace network {
    
HttpTask::HttpTask(const std::string& name) :
task_name(name){
    task_id = base::GenerateGUID();
    socket_watcher_timeout = 0;
    is_post = true;
}
    
HttpTask::~HttpTask() {
}
    
void HttpTask::ProgressFeedBack(network::BaseConnection* connection, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow) {
    ;
}
    
int HttpTask::OnResponse(network::ProtocolErrorCode error_code,
                   const HTTP_HEADERS& headers,
                         const std::string& resp, long http_code) {
    return 0;
}

HttpCgiTaskDispatcher::HttpCgiTaskDispatcher(const std::string& name,
                                                 bool fifo,
                                                 int connection_count) :
AsyncTaskDispatcher(name, fifo) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  CreateFixedConnections(connection_count);
}

HttpCgiTaskDispatcher::~HttpCgiTaskDispatcher(){
    printf("HttpCgiTaskDispatcher::~HttpCgiTaskDispatcher");
}

void HttpCgiTaskDispatcher::ScheduleTask(const scoped_refptr<HttpTask>& task,
                                           const HandlerCallback& handler_callback) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  ScheduleTasksWithPriority(base::Bind(&HttpCgiTaskDispatcher::RunTask,
                                         this,
                                         task,
                                         handler_callback),
                              1);
  
}

void HttpCgiTaskDispatcher::RunTask(const scoped_refptr<HttpTask>& task,
                                      const HandlerCallback& handler_callback) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  ConfigTaskAndConnection(task, handler_callback);
}


void HttpCgiTaskDispatcher::ConfigTaskAndConnection(const scoped_refptr<HttpTask>& task,
                                                     const HandlerCallback& handler_callback) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    CurlConnection* connection = GetFlaggedConnection() ;
    if (NULL == connection) {
        LOG(ERROR) << "HttpCgiTaskDispatcher ConfigTaskAndConnection GetFlaggedConnection pConnection is NULL!";
        NOTREACHED();
    }
    if(!connection->Init(true)) {
        LOG(ERROR) << "HttpCgiTaskDispatcher ConfigTaskAndConnection pConnection init failed!";
        NOTREACHED();
    }
    connection->SetUrl(task->url);
    if(task->headers.size()>0){
        connection->SetHttpHeaders(task->headers);
    }
    if(task->upload_content.length() > 0) {
        connection->SetBufferReadContent(new network::NetWorkMemoryBuffer(task->upload_content));
    } else if(task->upload_path.length() > 0){
        connection->SetBufferReadContent(new network::NetWorkFileBuffer(task->upload_path,false,false));
    }
    connection->SetSocketWatcherTimeoutMs(task->socket_watcher_timeout);
    if(task->download_path.length() > 0){
        connection->SetBufferWriteContent(new network::NetWorkFileBuffer(task->download_path,false,false));
    }
    connection->SetPost(task->is_post);
    connection->SetProgressCallback(base::Bind(&HttpTask::ProgressFeedBack, task));
    ConfigCompleteCallback(task, connection, handler_callback);
    AsyncProcess(connection);
}

static size_t CalcHttpHeaderBytesSize(const std::list<std::string>& headers) {
  size_t size = 0;
  
  for (auto& h : headers) {
    size += h.length() + 2;
  }
  
  size += 74; // 经验值
  
  return size;
}
  
void HttpCgiTaskDispatcher::OnCompleteCallback(
    const scoped_refptr<HttpTask>& task,
    CurlConnection* connection,
    const HandlerCallback& handler_callback,
    network::ProtocolErrorCode error_code) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  
    uint64_t req_size = connection->GetBufferRead()->getContentSize() + CalcHttpHeaderBytesSize(connection->GetRequestHttpHeaders());
    uint64_t resp_size = connection->GetBufferWrite()->getCompletedSize() + connection->GetRawHeader().size();
#ifdef PERFORMTEST
    LOG(WARNING) << "[TrafficLOG] NetType: " << g_net_type << " " << task->TaskName() << " req size: " << req_size << " resp size: " << resp_size;
#endif
    if(error_code != PEC_OK) {
        if (!handler_callback.is_null()) {
          handler_callback.Run(error_code, HTTP_HEADERS(), "");
        } else {
          task->OnResponse(error_code, HTTP_HEADERS(), "", 0);
        }
        ReleaseBusyConnection(connection);
        return;
    }
    ParseResponse(task, connection, handler_callback);
}

void HttpCgiTaskDispatcher::ConfigCompleteCallback(
    const scoped_refptr<HttpTask>& task,
    CurlConnection* connection,
    const HandlerCallback& handler_callback) {
  DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
  auto complete_callback = base::Bind(&HttpCgiTaskDispatcher::OnCompleteCallback,
                                      this,
                                      task,
                                      connection,
                                      handler_callback);
  
  CreateSocketWatcher(connection, complete_callback);
}

int HttpCgiTaskDispatcher::ParseResponse(const scoped_refptr<HttpTask>& task,
                                           CurlConnection* pConnection,
                                           const HandlerCallback& handler_callback) {
    DCHECK(BusinessThread::CurrentlyOn(BusinessThread::IO));
    long response_code = pConnection->GetHttpCode();
    HTTP_HEADERS headers = pConnection->GetHttpHeaders();
    const std::string& original = pConnection->GetBufferWrite()->getContent();
    int error = 0;

    if (response_code >= 200 && response_code < 300) {
        // 让回调去做实际的解析
        int ret = 0;
        if (!handler_callback.is_null()) {
          ret = handler_callback.Run(network::PEC_OK, headers, original);
        } else {
          ret = task->OnResponse(network::PEC_OK, headers, original, response_code);
        }

        if (ret) {
          error = 1;
        }
    }
    else {
        LOG(ERROR) << "Http response error: " << response_code << ", " << task->task_name;
        LOG(ERROR) << "headers :";
        for (auto& it : headers) {
            LOG(ERROR) << it.first << " : " << it.second;
        }
        
        LOG(ERROR) << "response is : " << original;
        
        if (!handler_callback.is_null()) {
          handler_callback.Run(network::PEC_FAILED_RESPONSE, headers, original);
        } else {
          task->OnResponse(network::PEC_FAILED_RESPONSE, headers, original, response_code);
        }
        error = 4;
    }
    
    ReleaseBusyConnection(pConnection);
    return error;
}

}
