#ifndef base_connection_h
#define base_connection_h

#include "network_define.h"

namespace network {
  class BaseConnection {
  public:
    virtual ~BaseConnection(){};
    // getters & setters
    virtual const NetWorkBuffer* GetBufferRead() = 0;
    virtual void SetBufferReadContent(NetWorkBuffer* buffer_read) = 0;
    virtual const NetWorkBuffer* GetBufferWrite() = 0;
    virtual void SetBufferWriteContent(NetWorkBuffer* buffer_write) = 0;
    virtual void SetCompleteCallback(const CompleteCallback& callback) = 0;
    virtual void SetProgressCallback(const ProgressCallback& callback) = 0;
    virtual void SetAbortCallback(const TaskAbortCallback& callback) = 0;
    virtual void SetSocketWatcherTimeoutMs(int64_t timeout_ms) = 0;
  };
  
  class BaseConnectionImpl : public BaseConnection {
  public:
    BaseConnectionImpl() : socket_watcher_timeout_ms_(0) { }
    // getters & setters
    virtual const NetWorkBuffer* GetBufferRead() override {
      return buffer_read_.get();
    }
    virtual void SetBufferReadContent(NetWorkBuffer* buffer_read) override {
      buffer_read_.reset(buffer_read);
    }
    virtual const NetWorkBuffer* GetBufferWrite() override {
      return buffer_write_.get();
    }
    virtual void SetBufferWriteContent(NetWorkBuffer* buffer_write) override {
      buffer_write_.reset(buffer_write);
    }
    virtual void SetCompleteCallback(const CompleteCallback& callback) override {
      complete_callback_ = callback;
    }
    virtual void SetProgressCallback(const ProgressCallback &callback) override {
      progress_callback_ = callback;
    }
    virtual void SetAbortCallback(const TaskAbortCallback& callback) override {
      abort_callback_ = callback;
    }
    virtual void SetSocketWatcherTimeoutMs(int64_t timeout_ms) override {
      socket_watcher_timeout_ms_ = timeout_ms;
    }
  protected:
    void ResetCallbacks() {
      complete_callback_.Reset();
      progress_callback_.Reset();
      abort_callback_.Reset();
    }
    scoped_ptr<NetWorkBuffer> buffer_read_;
    scoped_ptr<NetWorkBuffer> buffer_write_;
    int64_t socket_watcher_timeout_ms_;
    CompleteCallback complete_callback_;
    ProgressCallback progress_callback_;
    TaskAbortCallback abort_callback_;
  };
}

#endif /* base_connection_h */
