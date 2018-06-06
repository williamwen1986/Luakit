#pragma once

#include "base/callback.h"
#include "base/timer/timer.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"
#include "network_define.h"

#define ASYNC_CURL_TIMEOUT 5000

namespace network {
  
    typedef enum __SocketEvent {
      Event_None  = 0,      // CURL_POLL_NONE
      Event_Read  = 1 << 0, // CURL_POLL_IN
      Event_Write = 1 << 1, // CURL_POLL_OUT
    } SocketEvent;
    typedef base::Callback<void(int socket, SocketEvent event)> SocketEventCallback;
    
//#ifdef OS_IOS
//    class SocketWatcher : public base::MessagePumpIOSForIO::Watcher, public base::RefCounted<SocketWatcher> {
//#else
    class SocketWatcher : public base::MessagePumpLibevent::Watcher, public base::RefCounted<SocketWatcher> {
//#endif
    public:
        SocketWatcher(const SocketEventCallback& callback) ;
        ~SocketWatcher();
        
        virtual void OnFileCanReadWithoutBlocking(int socket) ;
        virtual void OnFileCanWriteWithoutBlocking(int socket) ;
        
        void Init(int socket, base::MessageLoopForIO::Mode mode);
        /**
         * @brief 开始监听socket
         */
        void StartWatching();
        /**
         * @brief socket读写事件发生
         */
        void OnSocketEvent(int socket, SocketEvent event);
        /**
         * @brief 停止监听socket，并且关闭io timeout timer，整个流程结束
         */
        void FinishWatching();
        /**
         * @brief 停止监听socket，但是并不关闭io timeout timer
         * @note curl回调中socket发生改变，需要停止监听旧的socket，然后重新监听新的socket
         */
        void StopWatching();
      
        void SetTimeoutMs(int64_t timeout_ms);
        
    public:
        base::MessageLoopForIO::Mode watch_mode_ ;
        bool is_io_timeout_;
        
    private:
        void StartIOTimeoutTimer();
        void OnIOTimeout();
        void CallbackWrapper(int socket, SocketEvent event);
        
    private:
        int watching_socket_ ;
        int64_t timeout_ms_;
        SocketEventCallback event_callback_ ;
        base::WeakPtrFactory<SocketWatcher> weak_factory_;
        scoped_ptr<base::OneShotTimer<SocketWatcher> > io_timeout_timer_;
        base::MessageLoopForIO::FileDescriptorWatcher fd_watcher_ ;
    } ;

} // namespace network
