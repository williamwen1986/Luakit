#include "socket_watcher.h"

namespace network {
    SocketWatcher::SocketWatcher(const SocketEventCallback& callback) :
    is_io_timeout_(false),
    timeout_ms_(0),
    weak_factory_(this) {
        event_callback_ = callback;
        Init(0, base::MessageLoopForIO::WATCH_READ_WRITE) ;
    }
    
    SocketWatcher::~SocketWatcher() {
    }
    
    void SocketWatcher::Init(int socket, base::MessageLoopForIO::Mode mode) {
        watching_socket_ = socket;
        watch_mode_ = mode;
    }
    
    void SocketWatcher::StartWatching() {
        StartIOTimeoutTimer();
        base::MessageLoopForIO::current()->WatchFileDescriptor(watching_socket_,
                                                               true,
                                                               watch_mode_,
                                                               &fd_watcher_,
                                                               this) ;
    }
    
    void SocketWatcher::OnSocketEvent(int socket, SocketEvent event) {
        CallbackWrapper(socket, event);
    }
    
    void SocketWatcher::FinishWatching() {
        io_timeout_timer_.reset();
        is_io_timeout_ = false;
        fd_watcher_.StopWatchingFileDescriptor();
    }
    
    void SocketWatcher::StopWatching() {
        is_io_timeout_ = false;
        fd_watcher_.StopWatchingFileDescriptor();
    }
  
    void SocketWatcher::SetTimeoutMs(int64_t timeout_ms) {
        timeout_ms_ = timeout_ms;
    }
    
    void SocketWatcher::OnFileCanReadWithoutBlocking(int socket)
    {
        StartIOTimeoutTimer();
        OnSocketEvent(socket, Event_Read) ;
    }
    
    void SocketWatcher::OnFileCanWriteWithoutBlocking(int socket)
    {
        StartIOTimeoutTimer();
        OnSocketEvent(socket, Event_Write) ;
    }
    
    void SocketWatcher::StartIOTimeoutTimer() {
        is_io_timeout_ = false;
        io_timeout_timer_.reset(new base::OneShotTimer<SocketWatcher>());
        io_timeout_timer_->Start(FROM_HERE,
                                 base::TimeDelta::FromMilliseconds(timeout_ms_ == 0 ? ASYNC_CURL_TIMEOUT : timeout_ms_),
                                 this,
                                 &SocketWatcher::OnIOTimeout);
    }
    
    void SocketWatcher::OnIOTimeout() {
        LOG(ERROR) << "SocketWatcher IO timeout socket: " << watching_socket_;
        is_io_timeout_ = true;
        CallbackWrapper(watching_socket_, Event_None);
    }
    
    void SocketWatcher::CallbackWrapper(int socket, SocketEvent event) {
        scoped_refptr<SocketWatcher> AddRefForDontCreateWatcherInFollowingMfRun(this);
        // IF CREATE m_pWatcher = new SocketWatcher(f) in the event_callback_.Run(fd) the operator= will release the "this" ref once
        event_callback_.Run(socket, event);
        // IF CREATE m_pWatcher = new SocketWatcher(f) in the event_callback_.Run(fd) after the } "this" will release this ref once again and the "this" will be delete
        // IF NOOOOOOO CREATE m_pWatcher = new SocketWatcher(f) in the m_f.Run(fd) after the } this will be the same as before the {
    }
} // namespace network

