#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "curl_connection.h"
#include "network/async_task_dispatcher.h"
#include "common/base_lambda_support.h"
#include "base/thread_task_runner_handle.h"
bool OPEN_CURL_LOG = true;

namespace network {

    const static long CURLCONNECTION_TIMEOUT_MS = 15*1000; // 15s
    
    CurlConnection::CurlConnection(int nIndex, const std::string& name, bool auto_reset, AsyncTaskDispatcher* dispatcher):
    ignore_force_fail_(false),
    force_fail_(false),
    curl_code_(CURLE_OK),
    force_new_(false),
    auto_reset_(auto_reset),
    weakptr_factory_(this) {
        download_total_ = 0;
        download_now_ = 0;
        upload_total_ = 0;
        upload_now_ = 0;
        download_range_start_ = 0;
        download_content_length_ = 0;
        index_ = nIndex ;
        name_ = name;
        curl_easy_handler_ = NULL;
        curl_multi_handler_ = curl_multi_init();
        // share SSL Session
        curl_share_handler_ = curl_share_init();
        curl_share_setopt(curl_share_handler_, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        curl_headers_ = NULL;
        curl_resolve_ = NULL;
        socket_watcher_ = NULL;
        dispatcher_ = dispatcher;
        apply_status_ = APPLYSTATUS_IDLE;
    }

    CurlConnection::~CurlConnection() {
        timeout_timer_.Stop();
        if(socket_watcher_) {
            socket_watcher_->FinishWatching();
        }
        
        if(curl_headers_) {
            curl_slist_free_all(curl_headers_);
            curl_headers_ = NULL;
        }
      
        if(curl_resolve_) {
            curl_slist_free_all(curl_resolve_);
            curl_resolve_ = NULL;
        }
      
        if(curl_multi_handler_) {
            curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_);
            curl_multi_cleanup(curl_multi_handler_) ;
            curl_multi_handler_ = NULL;
        }
        if (curl_easy_handler_) {
            curl_easy_cleanup(curl_easy_handler_) ;
            curl_easy_handler_ = NULL;
        }
        if(curl_share_handler_) {
            curl_share_setopt(curl_share_handler_, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_SSL_SESSION);
            curl_share_cleanup(curl_share_handler_);
            curl_share_handler_ = NULL;
        }
    }

    bool CurlConnection::Init(bool need_read_func) {
        curl_code_ = CURLE_OK;
        if (NULL == curl_multi_handler_) {
          return false ;
        }
        if (NULL != curl_easy_handler_) {
            curl_easy_cleanup(curl_easy_handler_) ;
            curl_easy_handler_ = NULL;
        }
        curl_easy_handler_ = curl_easy_init() ;
        if (NULL == curl_easy_handler_) {
            return false ;
        }
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_SOCKETFUNCTION, CurlSocketCallback) ;
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_TIMERFUNCTION, CurlTimerCallback) ;
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_SOCKETDATA, this) ;
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_TIMERDATA, this) ;
        // Connection
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_MAXCONNECTS, 1L); // the size of connection cache
        curl_multi_setopt(curl_multi_handler_, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1L); // max simultaneously open connections
        
        if (need_read_func) {
          curl_easy_setopt(curl_easy_handler_, CURLOPT_READFUNCTION, CurlReadCallback);
          curl_easy_setopt(curl_easy_handler_, CURLOPT_READDATA, this);
        }
      
        // 注意重置last_recv_time_的时机
        const time_t TOO_OLD_THRESHOLD = 60; // 60秒后使用新连接
        bool too_old = (time(NULL) - last_recv_time_) > TOO_OLD_THRESHOLD;
//        LOG_IF(WARNING, too_old && auto_reset_) << "60秒后使用新连接";
      
        if(force_new_ || (too_old && auto_reset_)) {
            force_new_ = false;
          curl_easy_setopt(curl_easy_handler_, CURLOPT_FRESH_CONNECT, 1L);
        }
        curl_easy_setopt(curl_easy_handler_, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SEEKFUNCTION, CurlSeekCallback);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SEEKDATA, this);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_HEADERFUNCTION, CurlHeaderCallback);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_HEADERDATA, this);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_XFERINFOFUNCTION, CurlProgressCallback);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_XFERINFODATA, this);
        
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SSL_VERIFYPEER, 0L) ;
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SSL_VERIFYHOST, 0L) ;
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT) ;
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SSL_SESSIONID_CACHE, 0L) ;
        curl_easy_setopt(curl_easy_handler_, CURLOPT_CONNECTTIMEOUT_MS, CURLCONNECTION_TIMEOUT_MS);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_LOW_SPEED_TIME, 60L);
        curl_easy_setopt(curl_easy_handler_, CURLOPT_LOW_SPEED_LIMIT, 1L); // 默认60秒内传输速度小于 1 bytes/sec则失败. 如果需要修改请使用SetLowSpeedLimit
        curl_easy_setopt(curl_easy_handler_, CURLOPT_NOSIGNAL, 1L) ;

        curl_easy_setopt(curl_easy_handler_, CURLOPT_FOLLOWLOCATION, 1L);
      
        if(OPEN_CURL_LOG) {
            curl_easy_setopt(curl_easy_handler_, CURLOPT_VERBOSE, 1L) ;
            curl_easy_setopt(curl_easy_handler_, CURLOPT_DEBUGFUNCTION, DebugCallback) ;
            curl_easy_setopt(curl_easy_handler_, CURLOPT_DEBUGDATA, this);
        }
      
        // 重置last_recv_time_
        last_recv_time_ = time(NULL);
      
        request_headers_.clear();

        // 默认读写内存缓存
        buffer_write_.reset(new NetWorkMemoryBuffer());
        buffer_read_.reset(new NetWorkMemoryBuffer());
        return true ;
    }
    
    void CurlConnection::CreateSocketWatcher(const CompleteCallback &complete_callback) {
        complete_callback_ = complete_callback;
        socket_watcher_ = new SocketWatcher(base::Bind(&CurlConnection::SocketEventCallback,
                                                       this));
        socket_watcher_->SetTimeoutMs(socket_watcher_timeout_ms_);
    }
    
    ProtocolErrorCode CurlConnection::GetCurlErrorCode(CURLcode curl_code) {
        ProtocolErrorCode error_code = PEC_UNKNOW_ERROR;
        switch(curl_code) {
            case CURLE_OK: {
                error_code = PEC_OK;
                break;
            }
            case CURLE_FTP_WEIRD_SERVER_REPLY: {
                error_code = PEC_WEIRD_SERVER_REPLY;
                break;
            }
            case CURLE_UNSUPPORTED_PROTOCOL: {
                error_code = PEC_UNSUPPORTED_PROTOCOL;
                break;
            }
            case CURLE_FAILED_INIT: {
                error_code = PEC_FAILED_INIT;
                break;
            }
            case CURLE_OPERATION_TIMEDOUT: {
                error_code = PEC_OPERATION_TIMEOUT;
                break;
            }
            case CURLE_LOGIN_DENIED: {
                error_code = PEC_AUTH_ERROR;
                break;
            }
            case CURLE_COULDNT_RESOLVE_PROXY:
            case CURLE_COULDNT_RESOLVE_HOST: {
                error_code = PEC_RESOLVE_ERROR;
                break;
            }
            case CURLE_SSL_CONNECT_ERROR: {
                error_code = PEC_SSL_ERROR;
                break;
            }
            case CURLE_COULDNT_CONNECT: {
                error_code = PEC_COULDNT_CONNECT;
                break;
            }
            case CURLE_SEND_ERROR:
            case CURLE_RECV_ERROR: {
                error_code = PEC_NETWORK_ERROR;
                break;
            }
            case CURLE_ABORTED_BY_CALLBACK: {
                error_code = PEC_ABORTED;
                break;
            }
            default: {
                LOG(ERROR) << "ProtocolHelper GetCurlErrorCode unknown curl code: " << curl_code;
                error_code = PEC_UNKNOW_ERROR;
                break;
            }
        }
        
        return error_code;
    }
    
    ProtocolErrorCode CurlConnection::CheckSocketCompleted(int socket, SocketEvent event) {
      if (-1 == socket) {
        LOG(ERROR) << name_ << " CurlConnection CheckSocketCompleted socket is invalid! curl code: " << curl_code_;
        CURLMcode mcode = curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_) ;
        LOG_IF(ERROR, mcode != CURLM_OK) << name_ << " CurlConnection CheckSocketCompleted socket is invalid curl_multi_remove_handle failed! mcode: " << mcode;
        ProtocolErrorCode code = GetCurlErrorCode(curl_code_);
        return code == PEC_OK ? PEC_NETWORK_ERROR : code ;
      }
      
      if (socket_watcher_->is_io_timeout_) {
        LOG(ERROR) << name_ << " CurlConnection CheckSocketCompleted IO timeout! completed size: " << buffer_write_->getCompletedSize() << " bytes!";
        CURLMcode mcode = curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_) ;
        LOG_IF(ERROR, mcode != CURLM_OK) << name_ << " CurlConnection CheckSocketCompleted IO timeout curl_multi_remove_handle failed! mcode: " << mcode;
        socket_watcher_->FinishWatching() ;
        return PEC_OPERATION_TIMEOUT ;
      }

      int nStillRunning = 0 ;
      DCHECK(event != Event_None) << "执行到这个地方应该有读写事件发生!";
      int action = (event&Event_Read?CURL_POLL_IN:0)|(event&Event_Write?CURL_POLL_OUT:0);
      CURLMcode rc = curl_multi_socket_action(curl_multi_handler_,
                                              socket,
                                              action,
                                              &nStillRunning) ;
      if(rc != CURLM_OK) {
        LOG(ERROR) << name_ << " CurlConnection CheckSocketCompleted curl_multi_socket_action failed! mcode: " << rc;
        curl_code_ = curl_code_ != CURLE_OK ? curl_code_ : CURLE_FAILED_INIT;
      }
      if (nStillRunning > 0) {
        return PEC_NOT_COMPLETE ;
      }
      if (curl_code_ != CURLE_OK) {
        LOG(ERROR) << name_ << " CurlConnection CheckSocketCompleted failed! curl_code_: " << curl_code_;
        CURLMcode mcode = curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_) ;
        LOG_IF(ERROR, mcode != CURLM_OK) << name_ << " CurlConnection CheckSocketCompleted curl_multi_remove_handle failed! mcode: " << mcode;
        socket_watcher_->FinishWatching() ;
        return PEC_UNKNOW_ERROR ;
      }
      
      return ReadCurlMultiInfo();
    }
    
    CURLMcode CurlConnection::Perform() {
        return curl_multi_add_handle(curl_multi_handler_, curl_easy_handler_) ;
    }
    
    void CurlConnection::Clean() {
        url_.clear();
        host_.clear();
        port_.clear();
        download_total_ = 0;
        download_now_ = 0;
        upload_total_ = 0;
        upload_now_ = 0;
        download_range_start_ = 0;
        download_content_length_ = 0;
        ignore_force_fail_ = false;
        force_fail_ = false;
        socket_watcher_timeout_ms_ = 0;
        buffer_read_->clean() ;
        buffer_write_->clean() ;
        socket_watcher_ = NULL;
        curl_code_ = CURLE_OK;
        curl_errmsg_.clear();
        response_headers_.clear();
        complete_callback_.Reset();
        progress_callback_.Reset();
        abort_callback_.Reset();
        if (curl_headers_) {
            curl_slist_free_all(curl_headers_);
            curl_headers_ = NULL;
        }
        if(curl_resolve_) {
            curl_slist_free_all(curl_resolve_);
            curl_resolve_ = NULL;
        }
        // clean share SSL Session
        curl_easy_setopt(curl_easy_handler_, CURLOPT_SHARE, NULL);
    }
    
    void CurlConnection::ForceFinish(bool finish_watching) {
        if(finish_watching) {
            socket_watcher_->FinishWatching();
        }
        socket_watcher_->OnSocketEvent(-1, Event_None);
    }

    void CurlConnection::SetUrl(const std::string& url) {
        url_ = url;
        curl_easy_setopt(curl_easy_handler_, CURLOPT_URL, url.c_str());
        LOG(WARNING) << "Curl SetUrl: " << url;
    }
  
    void CurlConnection::SetUserAndPassword(const std::string &user, const std::string &pwd) {
      if(!user.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_USERNAME, user.c_str());
      }
      if(!pwd.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_PASSWORD, pwd.c_str());
      }
    }
  
    void CurlConnection::SetUserAndXOAuth2Bearer(const std::string& user, const std::string& bearer) {
      if(!user.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_USERNAME, user.c_str());
      }
      if(!bearer.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_XOAUTH2_BEARER, bearer.c_str());
      }
    }
  
    void CurlConnection::SetCustomRequest(const std::string &value) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_CUSTOMREQUEST, value.c_str());
    }
    
    void CurlConnection::SetPrivateData(const std::string &value) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_PRIVATE, value.c_str());
    }
  
    void CurlConnection::SetConnectTimeout(const int timeout_ms) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    }
  
    void CurlConnection::SetFreshConnection() {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_FRESH_CONNECT, 1L);
    }
  
    void CurlConnection::SetForbidReuse() {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_FORBID_REUSE, 1L);
    }
  
    void CurlConnection::SetConnectOnly() {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_CONNECT_ONLY, 1L);
    }
  
    void CurlConnection::SetLowSpeedLimit(const int64_t time, const int64_t speed) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_LOW_SPEED_TIME, time);
      curl_easy_setopt(curl_easy_handler_, CURLOPT_LOW_SPEED_LIMIT, speed);
    }
  
    void CurlConnection::SetTryToUseSSL() {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    }
  
    void CurlConnection::SetMailFrom(const std::string& from) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_MAIL_FROM, from.c_str());
    }
    
    void CurlConnection::SetMailRcpt(const std::list<std::string>& to_list) {
      for(auto& to : to_list) {
        curl_headers_ = curl_slist_append(curl_headers_, to.c_str());
      }
      curl_easy_setopt(curl_easy_handler_, CURLOPT_MAIL_RCPT, curl_headers_);
      curl_easy_setopt(curl_easy_handler_, CURLOPT_UPLOAD, 1L);
    }
  
    void CurlConnection::SetAgentInfo(const std::string &value) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_USERAGENT, value.c_str());
    }
  
    void CurlConnection::SetProxy(const NetworkProxyInfo& info) {
      switch (info.type_) {
        case PROXY_HTTP: {
          std::string server("http://") ;
          server.append(info.server_) ;
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXY, server.c_str());
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
          curl_easy_setopt(curl_easy_handler_, CURLOPT_HTTPPROXYTUNNEL, 1L);
          break;
        }
        case PROXY_SOCKS4: {
          std::string server("socks4://");
          server.append(info.server_);
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXY, server.c_str());
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
          break;
        }
        case PROXY_SOCKS5: {
          std::string server("socks5://");
          server.append(info.server_);
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXY, server.c_str());
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
          break;
        }
        case PROXY_SOCKS5H: {
          std::string server("socks5h://");
          server.append(info.server_);
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXY, server.c_str());
          curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
          break;
        }
        default: {
          return;
        }
      }
      curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYPORT, info.port_);
      if(!info.username_.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYUSERNAME, info.username_.c_str());
      }
      if(!info.password_.empty()) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_PROXYPASSWORD, info.password_.c_str());
      }
    }
  
    void CurlConnection::SetEnableCompress(bool value) {
      curl_easy_setopt(curl_easy_handler_, CURLOPT_ACCEPT_ENCODING, value ? "" : NULL); // 设置空字符串表示接受curl支持的所有压缩算法
    }

    void CurlConnection::SetShareSSLSession(bool value) {
        if(value) {
            curl_easy_setopt(curl_easy_handler_, CURLOPT_SHARE, curl_share_handler_);
        }
        else {
            curl_easy_setopt(curl_easy_handler_, CURLOPT_SHARE, NULL);
        }
    }
    void CurlConnection::SetPost(bool post) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_POST, post ? 1L : 0L);
    }
    void CurlConnection::SetPostFields(const std::string &post_fields) {
        curl_easy_setopt(curl_easy_handler_, CURLOPT_POSTFIELDS, post_fields.c_str());
        curl_easy_setopt(curl_easy_handler_, CURLOPT_POSTFIELDSIZE, post_fields.size());
    }

    void CurlConnection::SetPostForm(network::CurlHttpForm *form) {
        form_.reset(form);

        if (form_.get()) {
          curl_easy_setopt(curl_easy_handler_, CURLOPT_HTTPPOST, form_->GetForm());
        }
    }

    void CurlConnection::SetHttpHeaders(const std::list<std::pair<std::string, std::string>>& http_headers) {
        request_headers_.clear();
        for(auto& it : http_headers) {
            if("Content-Length" == it.first) {
                base::StringToInt64(it.second, &upload_total_);
            }
            std::string header;
            header = (it.first + ": " + it.second);
            curl_headers_ = curl_slist_append(curl_headers_, header.c_str());
            request_headers_.push_back(header);
        }
        curl_easy_setopt(curl_easy_handler_, CURLOPT_HTTPHEADER, curl_headers_) ;
    }
    void CurlConnection::AddHttpHeader(const std::string& field, const std::string& value) {
        if("Content-Length" == field) {
          base::StringToInt64(value, &upload_total_);
        }
        std::string header = field + ": " + value;
        curl_headers_ = curl_slist_append(curl_headers_, header.c_str());
        curl_easy_setopt(curl_easy_handler_, CURLOPT_HTTPHEADER, curl_headers_) ;
        request_headers_.push_back(header);
    }
    long CurlConnection::GetHttpCode() {
        long code = 0;
        curl_easy_getinfo(curl_easy_handler_, CURLINFO_RESPONSE_CODE, &code);
        return code;
    }

    const std::string& CurlConnection::GetRawHeader() const {
        return response_headers_;
    }
    
    std::list<std::pair<std::string, std::string>> CurlConnection::GetHttpHeaders() {
        return ParseHttpHeaders();
    }
    
    std::list<std::pair<std::string, std::string>> CurlConnection::ParseHttpHeaders() {
        std::list<std::pair<std::string , std::string> > http_headers;
        std::vector<std::string> header_lines;
        base::SplitStringUsingSubstr(response_headers_, "\r\n", &header_lines);
        
        for (auto& each_header_line : header_lines) {
            if(!each_header_line.empty()) {
                // 第一个":"前面的是key，后面的是value
                size_t index = each_header_line.find(":");
                if(index != std::string::npos) {
                    char trim_char[] = " ";
                    std::string trim_value;
                    std::string key = each_header_line.substr(0, index);
                    base::TrimString(key, trim_char, &trim_value);
                    key = trim_value;
                    std::string value = each_header_line.substr(index+1, each_header_line.size() - (index+1));
                    base::TrimString(value, trim_char, &trim_value);
                    value = trim_value;
                    http_headers.push_back(std::make_pair(key, value));
                }
            }
        }
        return http_headers;
    }
  
#pragma mark - curl callback
    void CurlConnection::OnCurlSocketCallback(CURL *easy, curl_socket_t s, int what, void *socketp) {
      if(what == CURL_POLL_REMOVE) {
        if(!name_.empty()) {
          LOG(INFO) << name_ << " curlconnection socket finish watching! socket: " << s; // just for logging
        }
        socket_watcher_->FinishWatching();
        return;
      }
      if (NULL == socketp) {
        if(!name_.empty()) {
          LOG(INFO) << name_ << " curlconnection socket start watching! socket: " << s; // just for logging
        }
        curl_multi_assign(curl_multi_handler_, s, dispatcher_) ;
      } else {
        socket_watcher_->FinishWatching() ;
      }
      base::MessageLoopForIO::Mode mode ;
      if(ConvertMode(what, s, mode)) {
        socket_watcher_->Init(s, mode) ;
        socket_watcher_->StartWatching() ;
      }
    }
    
    void CurlConnection::OnCurlTimerCallback(CURLM *multi, long timeout_ms) {
      timeout_timer_.Stop();
      LOG_IF(INFO, timeout_ms == 0) << "[DEBUG]CurlConnection CurlTimer Start! 时间: 0ms";
      if (timeout_ms >= 0) {
        auto closure = base::Bind(&CurlConnection::OnCurlTimerTimeout, weakptr_factory_.GetWeakPtr(), timeout_ms);
        timeout_timer_.Start(FROM_HERE,
                             base::TimeDelta::FromMilliseconds(timeout_ms),
                             closure);
      }
    }
    
    void CurlConnection::OnCurlTimerTimeout(long timeout_ms) {
      LOG_IF(INFO, timeout_ms == 0) << "[DEBUG]CurlConnection CurlTimer Running! 时间: 0ms";
      int nStillRunning = 0 ;
      CURLMcode rc = curl_multi_socket_action(curl_multi_handler_, CURL_SOCKET_TIMEOUT, 0, &nStillRunning) ;
      if (rc > CURLM_OK) {
        // curl发生了不可逆的错误
        LOG(ERROR) << name_ << " CurlConnection TimerCallback curl_multi_socket_action fatal error: " << rc;
        rc = curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_) ;
        LOG_IF(ERROR, rc != CURLM_OK) << name_ << " CurlConnection TimerCallback curl_multi_remove_handle failed! mcode: " << rc;
        curl_code_ = CURLE_FAILED_INIT;
        return ;
      }
      if(nStillRunning > 0) {
        return;
      }
      ProtocolErrorCode pec = ReadCurlMultiInfo();
      DoCompleteCallback(pec);
    }
    
    void CurlConnection::OnCurlReadCallback(char* buffer, size_t& read_length) {
        buffer_read_->read(buffer, read_length);
    }
    
    bool CurlConnection::OnCurlWriteCallback(char* ptr, size_t size, size_t nmemb) {
      if(ptr) {
        last_recv_time_ = time(NULL);
        if(!force_fail_ || ignore_force_fail_) {
          buffer_write_->append(ptr, size*nmemb);
          return true;
        } else {
          LOG(ERROR) << name_ << " CurlConnection OnCurlWriteCallback 强制失败标志为true，数据不会写入本地!";
        }
      }
      return false;
    }
    
    void CurlConnection::OnCurlHeaderCallback(char* buffer, size_t length) {
        if (buffer && length > 0) {
            long code = GetHttpCode();
            // 设置了Range，判断服务器是否支持
            if (download_range_start_) {
                if (code >= 200 && code < 300) {
                    if (code != 206) {
                        LOG(ERROR) << name_ << " 带Range头的HTTP请求返回码不是206，必须重新下载！";
                        download_content_length_ += download_range_start_;
                        download_range_start_ = 0;
                        buffer_write_->setOffset(0);
                    }
                }
            }
            
            // 设置了目标内容大小，判断返回的ContentLength是否和期待的一致
            if (download_content_length_) {
                if (code != 301 && code != 302 && code != 303) {
                    std::string header(buffer, length);
                    std::size_t found = header.find("Content-Length:");
                    if(found != std::string::npos) {
                        std::size_t found2 = header.find("\r", found+1);
                        if(found2 != std::string::npos) {
                            base::StringToInt64(header.substr(found+strlen("Content-Length:"), found2-strlen("Content-Length:")), &download_total_);
                            // 检查设置的Content-Length和Http头返回的Content-Length是否一致
                            if(download_total_ != download_content_length_) {
                                LOG(ERROR) << name_ << " Http Resp Header Content-Length与设置值不一致，强制失败! Http Resp Header Content-Length: " << download_total_ << " 设置值: " << download_content_length_;
                                force_fail_ = true;
                            }
                        }
                    }
                    
                    found = header.find("Content-Encoding:");
                    if(found != std::string::npos) {
                        std::size_t found2 = header.find("\r", found+1);
                        if(found2 != std::string::npos) {
                            std::string encoding = header.substr(found+strlen("Content-Encoding:"), found2-strlen("Content-Encoding:"));
                            LOG(ERROR) << name_ << " Http Resp Header 设置了Content-Encoding，忽略强制失败的设置! Http Resp Header Content-Encoding: " << encoding;
                            ignore_force_fail_ = true;
                        }
                    }
                }
            }
            
            if (code != 301 && code != 302 && code != 303) {
                response_headers_.append(buffer, length);
            }
        }
    }
    
    bool CurlConnection::OnCurlSeekCallback(int64_t offset, int origin) {
        int64_t origin_offset = 0;
        switch (origin) {
            case SEEK_SET: {
                origin_offset = 0;
                break;
            }
            case SEEK_CUR: {
                origin_offset = buffer_read_->getOffset();
                break;
            }
            case SEEK_END: {
                origin_offset = buffer_read_->getContentSize();
                break;
            }
            default:
                return false;
        }
        int64_t new_offseet = origin_offset + offset;
        if (new_offseet < 0 ||
            new_offseet >= buffer_read_->getContentSize()) {
            return false;
        }
        buffer_read_->setOffset(new_offseet);
        return true;
    }
    
    bool CurlConnection::OnCurlProgressCallback() {
        if(!abort_callback_.is_null()) {
            return abort_callback_.Run();
        }
        return false;
    }
  
    void CurlConnection::OnSocketEventCallback(int socket, SocketEvent event) {
        ProtocolErrorCode pec = CheckSocketCompleted(socket, event) ;
        if(!progress_callback_.is_null()) {
            progress_callback_.Run(this, download_total_, download_now_, upload_total_, upload_now_);
        }
        if (PEC_NOT_COMPLETE == pec) {
            return ;
        }
        timeout_timer_.Stop();
        DoCompleteCallback(pec);
    }
  
    void CurlConnection::OnDebugCallback(curl_infotype type, const std::string& log) {
      if(type == CURLINFO_HEADER_IN){
      }else if (type == CURLINFO_HEADER_OUT){
      }else if (type == CURLINFO_DATA_IN){
      }else if (type == CURLINFO_DATA_OUT){
      }else if (type == CURLINFO_TEXT){
      }else if (type == CURLINFO_FAILED){
        LOG(ERROR) << name_ << " curl failed: " << log;
        curl_errmsg_ = log;
      }
    }
  
    void CurlConnection::GetHostAndPort(const std::string &url, std::string& host, std::string& port) {
      host.clear(); port.clear();
      if(!url.empty()) {
        size_t begin = url.find("://");
        if(begin != std::string::npos) {
          std::string scheam = url.substr(0, begin);
          if(scheam == "http") {
            port = "80";
          } else if(scheam == "https"){
            port = "443";
          }
          begin += 3;
          if(begin < url.size()) {
            size_t end = url.find("/", begin);
            if(end != std::string::npos) {
              host = url.substr(begin, end-begin);
            }
          }
        }
      }
    }
  
    bool CurlConnection::ConvertMode(int what, curl_socket_t s, base::MessageLoopForIO::Mode& mode) {
      if(CURL_POLL_NONE == what) {
        return false;
      } else if(CURL_POLL_INOUT == what) {
        LOG(INFO) << name_ << " CurlConnection SocketCallback CURL_POLL_INOUT FD: " << s;
        mode = base::MessageLoopForIO::WATCH_READ_WRITE ;
      } else if(CURL_POLL_IN == what) {
        LOG(INFO) << name_ << " CurlConnection SocketCallback CURL_POLL_IN FD: " << s;
        mode = base::MessageLoopForIO::WATCH_READ ;
      } else if(CURL_POLL_OUT == what) {
        LOG(INFO) << name_ << " CurlConnection SocketCallback CURL_POLL_OUT FD: " << s;
        mode = base::MessageLoopForIO::WATCH_WRITE ;
      } else if(CURL_POLL_REMOVE == what) {
        LOG(INFO) << name_ << " CurlConnection SocketCallback CURL_POLL_REMOVE FD: " << s;
        return false;
      }
      return true;
    }
  
    ProtocolErrorCode CurlConnection::ReadCurlMultiInfo() {
      ProtocolErrorCode ret = PEC_OK ;
      int msgs_in_queue = 0 ;
      CURLMsg* msg = curl_multi_info_read(curl_multi_handler_, &msgs_in_queue) ;
      if(msg && msg->msg == CURLMSG_DONE) {
        CURLcode res = msg->data.result ;
        ret = (CURLE_OK == res) ? PEC_OK : GetCurlErrorCode(res) ;
      }
      
      CURLMcode mcode = curl_multi_remove_handle(curl_multi_handler_, curl_easy_handler_) ;
      LOG_IF(ERROR, mcode != CURLM_OK) << name_ << " CurlConnection ReadCurlMultiInfo curl_multi_remove_handle failed! mcode: " << mcode;
      socket_watcher_->FinishWatching() ;
      return ret ;
    }
    
    void CurlConnection::DoCompleteCallback(ProtocolErrorCode err_code) {
        if(!complete_callback_.is_null()) {
            auto callback = complete_callback_;
            callback.Run(err_code);
        }
    }
    
#pragma mark - static callback
    int CurlConnection::CurlSocketCallback(CURL* easy, curl_socket_t s, int what, void* userp, void* socketp) {
        CurlConnection* pConnection = (CurlConnection*)userp ;
        if (NULL == pConnection) {
            NOTREACHED();
            LOG(ERROR) << "CurlConnection CurlSocketCallback connection is NULL!";
        }
        else {
            pConnection->OnCurlSocketCallback(easy, s, what, socketp);
        }
        return CURLM_OK;
    }
    
    int CurlConnection::CurlTimerCallback(CURLM *multi, long timeout_ms, void *userp) {
        CurlConnection* pConnection = (CurlConnection*)userp ;
        if (NULL == pConnection) {
            NOTREACHED();
            LOG(ERROR) << "CurlConnection CurlTimerCallback connection is NULL!";
        }
        else {
            pConnection->OnCurlTimerCallback(multi, timeout_ms);
        }
        return CURLM_OK;
    }
    
    size_t CurlConnection::CurlReadCallback(char *buffer, size_t size, size_t nitems, void *instream) {
        size_t ret = size * nitems ;
        if (ret < 1) {
            return ret ;
        }
        
        CurlConnection* pConnection = (CurlConnection*)instream ;
        if (NULL == pConnection) {
            NOTREACHED();
        }
        else {
            pConnection->OnCurlReadCallback(buffer, ret);
        }
        pConnection->upload_now_ += ret;
        return ret;
    }
    
    size_t CurlConnection::CurlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
        size_t ret = size * nmemb ;
        if (ret < 1) {
          return ret ;
        }
        CurlConnection* pConnection = (CurlConnection*)userdata ;
        if (NULL == pConnection) {
          NOTREACHED();
          LOG(ERROR) << "CurlConnection CurlWriteCallback CurlConnection NULL!";
          return -1;
        } else {
          if(pConnection->OnCurlWriteCallback(ptr, size, nmemb)) {
            pConnection->download_now_ += ret;
          } else {
            return -1;
          }
        }
        return ret;
    }
    
    size_t CurlConnection::CurlHeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
        size_t ret = size * nitems;
        CurlConnection* pConnection = (CurlConnection*)userdata;
        if(NULL == pConnection) {
            NOTREACHED();
        }
        else {
            pConnection->OnCurlHeaderCallback(buffer, ret);
        }
        return ret;
    }
    
    int CurlConnection::CurlSeekCallback(void *userp, curl_off_t offset, int origin) {
        CurlConnection* connection = (CurlConnection*)userp;
        if(connection->OnCurlSeekCallback((int)offset, origin)) {
            return 0;
        }
        return 1;
    }
    
    int CurlConnection::CurlProgressCallback(void *clientp,
                                             double dltotal,
                                             double dlnow,
                                             double ultotal,
                                             double ulnow) {
        CurlConnection* pConnection = (CurlConnection*)clientp;
        if(pConnection->OnCurlProgressCallback()) {
            return 1; // 返回非零值会让curl产生CURLE_ABORTED_BY_CALLBACK错误
        }
        return 0;
    }

    void CurlConnection::SocketEventCallback(void* connection, int socket, SocketEvent event) {
        CurlConnection* pConnection = (CurlConnection*)connection;
        if(NULL == pConnection) {
            NOTREACHED();
            LOG(ERROR) << "CurlConnection SocketEventCallback Connection is NULL!!!";
        } else {
            pConnection->OnSocketEventCallback(socket, event);
        }
    }
  
    int CurlConnection::DebugCallback(CURL* e,
                                      curl_infotype info_type,
                                      char * ptr,
                                      size_t size,
                                      void * whatever) {
      CurlConnection* pConnection = (CurlConnection*)whatever;
      if(pConnection != NULL && ptr != NULL && size > 0) {
        pConnection->OnDebugCallback(info_type, std::string(ptr, size));
      }
      return 0; // must be return 0
    }
}
