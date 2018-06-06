#pragma once

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_checker.h"
#include "net/io_buffer.h"
#include <netinet/in.h>
#include <sys/socket.h>

namespace net {
    
// A callback specialization that takes a single int parameter. Usually this is
// used to report a byte count or network error code.
typedef base::Callback<void(int)> CompletionCallback;
typedef base::Callback<void(bool)> BeforeConnectCallback;
typedef std::string sockaddr_data;
typedef base::Callback<std::vector<sockaddr_data>(const std::string&, uint16_t port)> DnsResolverCallback;

//#ifdef OS_IOS
//class AsyncSocket : public base::MessagePumpIOSForIO::Watcher {
//#else
class AsyncSocket : public base::MessagePumpLibevent::Watcher {
//#endif

public:
    static const std::string UseDefaultResolver;
    AsyncSocket(const sockaddr* addr, const socklen_t addrlen);
    AsyncSocket(const std::string& hostname, const uint16_t port, const DnsResolverCallback& dns_resolver_callback = DnsResolverCallback(), bool is_dns_resolver_callback_sync = false);
    ~AsyncSocket();
    
    int connect(const CompletionCallback& callback, const BeforeConnectCallback& beforeConnectionCallback = BeforeConnectCallback());
    int read(IOBuffer* buf, size_t buf_len, const CompletionCallback& callback);
    int write(IOBuffer* buf, size_t buf_len, const CompletionCallback& callback);
    void disconnect();
    bool isConnected() const;
    sockaddr_data currentSockaddr() const;
    
    virtual void OnFileCanReadWithoutBlocking(int fd);
    virtual void OnFileCanWriteWithoutBlocking(int fd);
    
    static DnsResolverCallback DefaultDnsResolver();
    static sockaddr_data IPV4toSockaddr(const std::string& ipv4string, uint16_t port);
    static sockaddr_data IPV6toSockaddr(const std::string& ipv6string, uint16_t port);
    static std::string Sockaddr2IP(const sockaddr* sockaddr);
  
private:
    int resolveHostBeforeConnect();
    void stopWatchingAndCleanUp();

    // State machine for connecting the socket.
    enum ConnectState {
        CONNECT_STATE_CONNECT,
        CONNECT_STATE_CONNECT_COMPLETE,
        CONNECT_STATE_NONE,
    };
    
    int socket_fd_;
    std::string hostname_;
    sockaddr_data hostaddr_;
    std::vector<sockaddr_data> sockaddrs_;
    uint16_t port_;
    ConnectState connect_state_;
    
    base::WeakPtrFactory<AsyncSocket> weak_factory_;
    base::MessageLoopForIO::FileDescriptorWatcher read_socket_watcher_;
    scoped_refptr<IOBuffer> read_buf_;
    size_t read_buf_len_;
    // External callback; called when read is complete.
    CompletionCallback read_callback_;
    
    base::MessageLoopForIO::FileDescriptorWatcher write_socket_watcher_;
    scoped_refptr<IOBuffer> write_buf_;
    size_t write_buf_len_;
    // External callback; called when write or connect is complete.
    CompletionCallback write_callback_;
    
    BeforeConnectCallback before_connect_callback_;
    
    scoped_ptr<base::Thread> dns_thread_;
    DnsResolverCallback dns_resolver_callback_;
    bool is_dns_resolver_callback_sync_;
    
private:
    base::ThreadChecker thread_checker_;
    
    DISALLOW_COPY_AND_ASSIGN(AsyncSocket);
};
}
