#pragma once

#include <string>
#include <vector>
#include <list>

#include "base/callback.h"
#include "base/file_util.h"
#include "base/memory/weak_ptr.h"
#include "base/path_service.h"
#include "base/platform_file.h"

typedef std::list<std::pair<std::string, std::string> > HTTP_HEADERS;

namespace network {
    
    enum ProtocolErrorCode {
        PEC_NOT_COMPLETE = -1,
        PEC_OK = 0,
        PEC_UNSUPPORTED_PROTOCOL,    // Unsupported protocol
        PEC_FAILED_INIT,             // Failed to init
        PEC_OPERATION_TIMEOUT,       // Operation timeout
        PEC_AUTH_ERROR,              // Authentication error
        PEC_NETWORK_ERROR,           // Network error
        PEC_INTERRUPT,               // Operation interrupt
        PEC_SSL_ERROR,               // SSL error
        PEC_SSL_VERSION_ERROR,       // SSL版本问题(error:14082174:SSL routines:ssl3_check_cert_and_algorithm:dh key too small)
        PEC_RESOLVE_ERROR,           // could not resolve server
        PEC_FAILED_RESPONSE,         // response error
        PEC_COULDNT_CONNECT,         // Couldn't connect
        PEC_ABORTED,                 // CURLE_ABORTED_BY_CALLBACK
        PEC_WEIRD_SERVER_REPLY,      // 服务器不按协议格式返回
        PEC_REQ_RSP_NOT_FIT,         // 请求item数量与返回item数量不一致
        PEC_UNKNOW_ERROR = 100,      // Unknown error
        PEC_LAST = 10000,            // Never use!
    };
    
    class BaseConnection;
    typedef base::Callback<void(ProtocolErrorCode)> CompleteCallback;
    typedef base::Callback<void(BaseConnection* connection, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow)> ProgressCallback;
    typedef base::Callback<bool()> TaskAbortCallback;
    typedef base::Callback<int(network::ProtocolErrorCode, const HTTP_HEADERS& headers, const std::string& resp)> HandlerCallback;
  
    typedef enum __NetWorkProxyType {
      PROXY_NOUSE = 0,
      PROXY_HTTP,
      PROXY_SOCKS4,
      PROXY_SOCKS5,
      PROXY_SOCKS5H //使用tencent后台提供的socks5代理请使用此type
    } NetWorkProxyType;
  
    class NetworkProxyInfo {
    public:
      NetworkProxyInfo() : port_(0), type_(PROXY_NOUSE) {}
      std::string server_;
      int32_t port_;
      std::string username_;
      std::string password_;
      NetWorkProxyType type_;
    };
  
    class NetWorkTask : public base::RefCounted<NetWorkTask> {
    public:
        NetWorkTask(int p, const base::Closure& closure)
        : priority_(p), closure_(closure) {}
        NetWorkTask() {}
        
    public:
        int priority_;
        base::Closure closure_;
    };
    
    class NetWorkBuffer : public base::SupportsWeakPtr<NetWorkBuffer> {
    public:
        NetWorkBuffer() : completed_size_(0) {}
        virtual ~NetWorkBuffer() {}
        /**
         * @brief 将读取到的内容写入buffer，read_length传入为最大读取大小，传出为实际读取大小
         */
        virtual void read(char* buffer, size_t& read_length) = 0;
        /**
         * @brief 将buffer中的内容写入缓存，length为buffer大小
         */
        virtual void append(char* buffer, size_t length) = 0;
        /**
         * @brief 返回缓存内容
         */
        virtual const std::string& getContent() const {return empty_content_;}
        /**
         * @brief 获取当前偏移量
         */
        virtual uint64_t getOffset() const = 0;
        /**
         * @brief 设置偏移量
         */
        virtual void setOffset(uint64_t offset) = 0;
        /**
         * @brief 获取buffer大小
         */
        virtual uint64_t getContentSize() const = 0;
        /**
         * @brief 获取已经完成收发的数据大小
         */
        virtual uint64_t getCompletedSize() const { return completed_size_; } // just for logging
        /**
         * @brief 清空缓存内容
         */
        virtual void clean() = 0;

      protected:
        std::string empty_content_;
        uint64_t completed_size_;
    };
    
    class NetWorkMemoryBuffer : public NetWorkBuffer {
    public:
        NetWorkMemoryBuffer() : NetWorkBuffer() {
            offset_ = 0;
        }
        NetWorkMemoryBuffer(const std::string& content) : NetWorkBuffer() {
            content_ = content;
            offset_ = 0;
        }
        virtual ~NetWorkMemoryBuffer() {}
        virtual void read(char* buffer, size_t& read_length) override {
            if(offset_ < content_.size()) {
                size_t len = std::min(read_length, content_.size() - (size_t)offset_) ;
                if (len > 0) {
                    memcpy(buffer, &(content_[offset_]), len) ;
                    offset_ += len ;
                    completed_size_ += len;
                    read_length = len;
                    return;
                }
            }
            read_length = 0;
        }
        virtual void append(char* buffer, size_t length) override {
            content_.append(buffer, length);
            completed_size_ += length;
        }
        virtual const std::string& getContent() const override {
            return content_;
        }
        virtual uint64_t getOffset() const override {
            return offset_;
        }
        virtual void setOffset(uint64_t offset) override {
            offset_ = offset;
        }
        virtual uint64_t getContentSize() const override {
            return content_.size();
        }
        virtual void clean() override {
            std::string().swap(content_);
            offset_ = 0;
            completed_size_ = 0;
        }
    private:
        std::string content_;
        std::string::size_type offset_;
    };
    
    class NetWorkFileBuffer : public NetWorkBuffer {
    public:
      explicit NetWorkFileBuffer(const std::string& path,
                                 bool read,
                                 bool cover,
                                 int64 offset = 0) : NetWorkBuffer() {
          read_ = read;
          stop_write_ = false;
          offset_ = offset;
          base::FilePath file_path(path);
          bool created = false;
          base::PlatformFileError error;
          int flags;
          if(read) {
            flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
          }else {
            flags = (cover ? base::PLATFORM_FILE_CREATE_ALWAYS : base::PLATFORM_FILE_OPEN_ALWAYS) | base::PLATFORM_FILE_WRITE | base::PLATFORM_FILE_APPEND;
          }
          platform_file_ = base::CreatePlatformFile(file_path,
                                                    flags,
                                                    &created,
                                                    &error);
          if(platform_file_ == base::kInvalidPlatformFileValue) {
              LOG(ERROR) << "Create File failed! error: " << error << " path: " << path;
              NOTREACHED();
              return;
          }
          LOG(INFO) << "跟踪FD使用情况! Opened FD: " << platform_file_;
          base::PlatformFileInfo file_info;
          if(base::GetPlatformFileInfo(platform_file_, &file_info)) {
              file_size_ = file_info.size;
          }
          else {
              LOG(ERROR) << "Get File info failed! path: " << path;
          }
      }
      virtual ~NetWorkFileBuffer() {
        if(platform_file_ != base::kInvalidPlatformFileValue) {
          if(!base::ClosePlatformFile(platform_file_)) {
            LOG(ERROR) << "NetworkFileBuffer destructor close platform file failed! FD: " << platform_file_;
          }
          platform_file_ = base::kInvalidPlatformFileValue;
        }
      }
      virtual void read(char* buffer, size_t& read_length) override {
          if(platform_file_ == base::kInvalidPlatformFileValue) {
              return;
          }
          if(offset_ < file_size_) {
              if (read_length > 0) {
                  int ret = base::ReadPlatformFile(platform_file_, offset_, buffer, read_length);
                  if(ret > 0) {
                      offset_ += ret;
                      completed_size_ += ret;
                      read_length = ret;
                      return;
                  }
              }
          }
          read_length = 0;
      }
      virtual void append(char* buffer, size_t length) override {
          completed_size_ += length;
          if(platform_file_ == base::kInvalidPlatformFileValue) {
              return;
          }
          if (!stop_write_) {
              base::WritePlatformFile(platform_file_, 0, buffer, length); // 如果用PLATFORM_FILE_APPEND方式打开文件，那么这个方法忽视offset
          }
      }
      virtual uint64_t getOffset() const override {
          return offset_;
      }
      virtual void setOffset(uint64_t offset) override {
          offset_ = offset;
          if (!stop_write_ && !read_) {
              base::TruncatePlatformFile(platform_file_, offset);
          }
      }
      virtual uint64_t getContentSize() const override {
          return file_size_;
      }
      virtual void clean() override {
          if(platform_file_ != base::kInvalidPlatformFileValue) {
            LOG(INFO) << "跟踪FD使用情况! Close FD: " << platform_file_;
            if(base::ClosePlatformFile(platform_file_)) {
              platform_file_ = base::kInvalidPlatformFileValue;
            } else {
              LOG(ERROR) << "NetworkFileBuffer clean() close platfom file failed! FD: " << platform_file_;
            }
          }
          offset_ = 0;
          file_size_ = 0;
          completed_size_ = 0;
      }
      // 因为NetWorkFileBuffer不能控制自己的生命周期，只好加个方法，调用后就不要写了，防止多个NetWorkFileBuffer同时往同一个文件里写造成内容异常
      virtual void stopWrite() {
          stop_write_ = true;
      }
    private:
      base::PlatformFile platform_file_;
      bool read_;
      std::string::size_type offset_;
      int64 file_size_;
      bool stop_write_;
    };
}
