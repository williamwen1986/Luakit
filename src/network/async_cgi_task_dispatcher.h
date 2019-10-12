#pragma once

#include "network/async_task_dispatcher.h"

namespace network {
    
class HttpTask : public base::RefCountedThreadSafe<HttpTask> {
    public:
    explicit HttpTask(const std::string& name);
    virtual ~HttpTask();
    std::string task_id;
    std::string task_name;
    std::string url;
    HTTP_HEADERS headers;
    bool is_post;
    //upload_content,upload_path二者选一
    std::string upload_content;
    std::string upload_path;
    std::string download_path;
    uint32_t socket_watcher_timeout;
    virtual void ProgressFeedBack(network::BaseConnection* connection, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow);
    virtual int OnResponse(network::ProtocolErrorCode error_code,
                           const HTTP_HEADERS& headers,
                           const std::string& resp, long http_code);
};


class HttpCgiTaskDispatcher : public AsyncTaskDispatcher {
public:
  explicit HttpCgiTaskDispatcher(const std::string& name,
                                   bool fifo = false,
                                   int connection_count = 5);
  virtual ~HttpCgiTaskDispatcher();
  
  virtual void ScheduleTask(const scoped_refptr<HttpTask>& task, const HandlerCallback& handler_callback = HandlerCallback());
  
protected:
  void ConfigTaskAndConnection(const scoped_refptr<HttpTask>& task, const HandlerCallback& handler_callback);
  int ParseResponse(const scoped_refptr<HttpTask>& task,
                    CurlConnection* pConnection,
                    const HandlerCallback& handler_callback);
  
  void RunTask(const scoped_refptr<HttpTask>& task, const HandlerCallback& handler_callback);
  
 private:
  void OnCompleteCallback(
    const scoped_refptr<HttpTask>& task,
    CurlConnection* pConnection,
    const HandlerCallback& handler_callback,
    network::ProtocolErrorCode error_code);
  
  void ConfigCompleteCallback(
    const scoped_refptr<HttpTask>& task,
    CurlConnection* pConnection,
    const HandlerCallback& handler_callback);

  void OnTaskAuthCallback(const scoped_refptr<HttpTask>& task, const HandlerCallback& handler_callback);
};

}
