#pragma once

#include <list>
#include <string>
#include "curl/curl.h"
#include "base/timer/timer.h"

#include "base_connection.h"
#include "socket_watcher.h"
#include "curl_http_form.h"

namespace network {
  class AsyncTaskDispatcher;
  class CurlConnection : public BaseConnectionImpl {
  public:
    enum APPLYSTATUS {
        APPLYSTATUS_BEGIN = 0,
        APPLYSTATUS_IDLE,
        APPLYSTATUS_FLAGGED,
        APPLYSTATUS_BUSY,
        APPLYSTATUS_END,
    } ;
    
    CurlConnection(int index, const std::string& name, bool auto_reset , AsyncTaskDispatcher* dispatcher);
    ~CurlConnection();
    
    bool Init(bool need_read_func = true);
    void CreateSocketWatcher(const CompleteCallback& complete_callback);
    ProtocolErrorCode GetCurlErrorCode(CURLcode curl_code);
    ProtocolErrorCode CheckSocketCompleted(int socket, SocketEvent event);
    CURLMcode Perform();
    void Clean();
    void ForceFinish(bool finish_watching);
    
    std::string Name() {return name_;}
    void SetUrl(const std::string& url);
    void SetUserAndPassword(const std::string& user, const std::string& pwd);
    void SetUserAndXOAuth2Bearer(const std::string& user, const std::string& bearer);
    void SetCustomRequest(const std::string& value);
    void SetPrivateData(const std::string& value);
    void SetConnectTimeout(const int timeout_ms);
    void SetFreshConnection();
    void SetForbidReuse();
    void SetConnectOnly();
    void SetLowSpeedLimit(const int64_t time/*second*/, const int64_t speed/*bytes per second*/);
    void SetTryToUseSSL();
    inline void SetDownloadTotal(const int64_t value) { download_total_ = value; }
    inline void SetDownloadNow(const int64_t value) { download_now_ = value; }
    inline void SetUploadTotal(const int64_t value) { upload_total_ = value; }
    inline void SetUploadNow(const int64_t value) { upload_now_ = value; }
    void SetMailFrom(const std::string& from);
    void SetMailRcpt(const std::list<std::string>& to_list);
    void SetAgentInfo(const std::string& value);
    void SetProxy(const NetworkProxyInfo& info);
    void SetEnableCompress(bool value);
    void SetShareSSLSession(bool value);
    void SetPost(bool post);
    void SetPostFields(const std::string& post_fields);

    void SetPostForm(CurlHttpForm* form);

    void SetHttpHeaders(const std::list<std::pair<std::string, std::string>>& http_headers);
    void AddHttpHeader(const std::string& field, const std::string& value);
    long GetHttpCode();
    std::string GetCurlErrMsg() const { return curl_errmsg_; }
    const std::string& GetRawHeader() const;
    std::list<std::pair<std::string, std::string>> GetHttpHeaders();
    const std::list<std::string>& GetRequestHttpHeaders() { return request_headers_; }
    
    inline APPLYSTATUS ApplyStatus() { return apply_status_; }
    inline void SetStatus(APPLYSTATUS status) { apply_status_ = status; }
    inline void SetForceNewConnection(const bool value = true) { force_new_ = value; }
  
    inline int64_t GetDownloadRangeStart() { return download_range_start_; }
    inline void SetDownloadRangeStart(int64_t value) { download_range_start_ = value; }
    inline void SetDownloadContentLength(int64_t value) { download_content_length_ = value; }
  
    inline CURLcode CurlCode() { return curl_code_; }
    
  protected:
    std::list<std::pair<std::string, std::string>> ParseHttpHeaders();
      
  protected:
    static int CurlSocketCallback(CURL *easy,      /* easy handle */
                                  curl_socket_t s, /* socket */
                                  int what,        /* see above */
                                  void *userp,     /* private callback pointer */
                                  void *socketp);  /* private socket pointer */
    static int CurlTimerCallback(CURLM *multi,     /* multi handle */
                                 long timeout_ms,  /* see above */
                                 void *userp);     /* private callback pointer */
    static size_t CurlReadCallback(char *buffer, size_t size, size_t nitems, void *instream);
    static size_t CurlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t CurlHeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);
    static int CurlSeekCallback(void *userp, curl_off_t offset, int origin);
    static int CurlProgressCallback(void *clientp, /* 目前仅仅用于中止连接 */
                                    double dltotal,
                                    double dlnow,
                                    double ultotal,
                                    double ulnow);
    static void SocketEventCallback(void* connection, int socket, SocketEvent event);
    static int DebugCallback (CURL* e, curl_infotype info_type, char * ptr, size_t size, void * whatever);
    
    void OnCurlSocketCallback(CURL *easy,
                              curl_socket_t s,
                              int what,
                              void *socketp);
    void OnCurlTimerCallback(CURLM *multi, long timeout_ms);
    void OnCurlTimerTimeout(long timeout_ms);
    void OnCurlReadCallback(char* buffer, size_t& read_length);
    bool OnCurlWriteCallback(char* ptr, size_t size, size_t nmemb);
    void OnCurlHeaderCallback(char* buffer, size_t length);
    bool OnCurlSeekCallback(int64_t offset, int origin);
    /**
     * @return true代表task被中止，需要停止传输数据，反之继续
     */
    bool OnCurlProgressCallback();
    void OnSocketEventCallback(int socket, SocketEvent event);
    void OnDebugCallback(curl_infotype type, const std::string& log);
      
  private:
    void GetHostAndPort(const std::string& url, std::string& host, std::string& port);
    bool ConvertMode(int what, curl_socket_t s, base::MessageLoopForIO::Mode& mode);
    ProtocolErrorCode ReadCurlMultiInfo();
    void DoCompleteCallback(ProtocolErrorCode err_code);
      
  private:
    int index_;
    std::string name_; // for logging
  
    std::string url_; // for logging
    std::string host_; // for logging
    std::string port_; // for logging
  
    // progress
    int64_t download_total_;
    int64_t download_now_;
    int64_t upload_total_;
    int64_t upload_now_;
  
    // 验证下载content-length
    int64_t download_range_start_;
    int64_t download_content_length_; // 如果http response header中content-length则认为回包非法，直接失败
    bool ignore_force_fail_; // 忽略下面设置的强制失败
    bool force_fail_; // 强制失败
  
    CURLcode    curl_code_;
    curl_slist* curl_headers_;
    curl_slist* curl_resolve_; // 用于设置host对应的ip
    CURL*       curl_easy_handler_;
    CURLM*      curl_multi_handler_;
    CURLSH*     curl_share_handler_;
    
    APPLYSTATUS apply_status_;

    scoped_ptr<CurlHttpForm> form_;
    
    bool force_new_; // 为true表示需要重新建立网络连接
    bool auto_reset_; // 是否在connection空闲一段时间后自动使用新连接
    
    std::string curl_errmsg_;
    
    scoped_refptr<SocketWatcher> socket_watcher_ ;
  
    base::OneShotTimer<CurlConnection> timeout_timer_;
    base::WeakPtrFactory<CurlConnection> weakptr_factory_;
    
    std::list<std::string> request_headers_;
      
    AsyncTaskDispatcher* dispatcher_ ;
      
    std::string response_headers_;
    
    time_t last_recv_time_;
    
  } ;
}
