#include "async_socket.h"
#include "base/callback_helpers.h"
#include "base/strings/string_number_conversions.h"
#include "common/base_lambda_support.h"
#include "net/net_errors.h"
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/tcp.h>
#if defined(OS_ANDROID)
#include <asm/fcntl.h>
#endif

using namespace net;

int SetNonBlocking(int fd) {
#if defined(OS_WIN)
    unsigned long no_block = 1;
    return ioctlsocket(fd, FIONBIO, &no_block);
#elif defined(OS_POSIX)
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags)
        return flags;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

bool SetTCPNoSigPipe(int fd) {
#if defined(OS_IOS)
  int set = 1;
  int error = setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));

  return error == 0;
#else
  return true;
#endif
}

// SetTCPNoDelay turns on/off buffering in the kernel. By default, TCP sockets
// will wait up to 200ms for more data to complete a packet before transmitting.
// After calling this function, the kernel will not wait. See TCP_NODELAY in
// `man 7 tcp`.
bool SetTCPNoDelay(int fd, bool no_delay) {
    int on = no_delay ? 1 : 0;
    int error = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    return error == 0;
}

// SetTCPKeepAlive sets SO_KEEPALIVE.
bool SetTCPKeepAlive(int fd, bool enable, int delay) {
    // Enabling TCP keepalives is the same on all platforms.
    int on = enable ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on))) {
        PLOG(ERROR) << "Failed to set SO_KEEPALIVE on fd: " << fd;
        return false;
    }
    
    // If we disabled TCP keep alive, our work is done here.
    if (!enable)
        return true;
    
#if defined(OS_LINUX) || defined(OS_ANDROID)
    // Setting the keepalive interval varies by platform.
    
    // Set seconds until first TCP keep alive.
    if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &delay, sizeof(delay))) {
        PLOG(ERROR) << "Failed to set TCP_KEEPIDLE on fd: " << fd;
        return false;
    }
    // Set seconds between TCP keep alives.
    if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &delay, sizeof(delay))) {
        PLOG(ERROR) << "Failed to set TCP_KEEPINTVL on fd: " << fd;
        return false;
    }
#elif defined(OS_MACOSX) || defined(OS_IOS)
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &delay, sizeof(delay))) {
        PLOG(ERROR) << "Failed to set TCP_KEEPALIVE on fd: " << fd;
        return false;
    }
#endif
    return true;
}

int MapAcceptError(int os_error) {
    switch (os_error) {
            // If the client aborts the connection before the server calls accept,
            // POSIX specifies accept should fail with ECONNABORTED. The server can
            // ignore the error and just call accept again, so we map the error to
            // ERR_IO_PENDING. See UNIX Network Programming, Vol. 1, 3rd Ed., Sec.
            // 5.11, "Connection Abort before accept Returns".
        case ECONNABORTED:
            return ERR_IO_PENDING;
        default:
            return MapSystemError(os_error);
    }
}

int MapConnectError(int os_error) {
    switch (os_error) {
        case EINPROGRESS:
            return ERR_IO_PENDING;
        case EACCES:
            return ERR_NETWORK_ACCESS_DENIED;
        case ETIMEDOUT:
            return ERR_CONNECTION_TIMED_OUT;
        default: {
            int net_error = MapSystemError(os_error);
            if (net_error == ERR_FAILED)
                return ERR_CONNECTION_FAILED;  // More specific than ERR_FAILED.
            return net_error;
        }
    }
}

size_t WriteWrapper(int fd, const void * buf, size_t size) {
#if defined(OS_ANDROID)
  int flags = MSG_NOSIGNAL;
#else
  int flags = 0;
#endif

  return ::send(fd, buf, size, flags);
}

const std::string AsyncSocket::UseDefaultResolver = "default";

//
// class AsyncSocket
//
AsyncSocket::AsyncSocket(const sockaddr* addr, const socklen_t addrlen)
    : socket_fd_(-1), hostaddr_((const char*)addr, addrlen),
      connect_state_(CONNECT_STATE_NONE), weak_factory_(this) {
    
}

AsyncSocket::AsyncSocket(const std::string& hostname, const uint16_t port, const DnsResolverCallback& dns_resolver_callback, bool is_dns_resolver_callback_sync)
    : socket_fd_(-1), hostname_(hostname), port_(port),
      connect_state_(CONNECT_STATE_NONE), weak_factory_(this),
      dns_resolver_callback_(dns_resolver_callback), is_dns_resolver_callback_sync_(is_dns_resolver_callback_sync) {
    
}

AsyncSocket::~AsyncSocket() {
    // dnsthread要先析构，因为dnsthread会在后台线程使用this的成员
    dns_thread_.reset();
    if (socket_fd_ > 0) {
        disconnect();
    }
}

int AsyncSocket::connect(const CompletionCallback& callback, const BeforeConnectCallback& beforeConnectCallback) {
    DCHECK(thread_checker_.CalledOnValidThread());
    DCHECK(connect_state_ == CONNECT_STATE_NONE);
    
    connect_state_ = CONNECT_STATE_CONNECT;
    write_callback_ = callback;
    before_connect_callback_ = beforeConnectCallback;
    
    if (hostname_.length() && sockaddrs_.empty()) {
        int ret = resolveHostBeforeConnect();
        if (ret != 0) return ERR_IO_PENDING;
    }
    
    while (!sockaddrs_.empty()) {
        hostaddr_ = sockaddrs_.front();
        sockaddrs_.erase(sockaddrs_.begin());
        
        struct sockaddr_storage *addr = (struct sockaddr_storage *)hostaddr_.data();
        LOG(WARNING) << "AsyncSocket connect IP: " << Sockaddr2IP((const sockaddr*)addr);
        socket_fd_ = socket(addr->ss_family, SOCK_STREAM, IPPROTO_TCP);
        if (socket_fd_ < 0) {
            switch errno {
            case EAFNOSUPPORT:
                LOG(ERROR) << "AsyncSocket create socket encounter EAFNOSUPPORT";
                break;
            case EPROTONOSUPPORT:
                LOG(ERROR) << "AsyncSocket create socket encounter EPROTONOSUPPORT";
                break;
            case ENFILE:
                LOG(ERROR) << "AsyncSocket create socket encounter ENFILE";
                break;
            case EMFILE:
                LOG(ERROR) << "AsyncSocket create socket encounter EMFILE";
                break;
            default:
                LOG(ERROR) << "AsyncSocket create socket returned an error, errno=" << errno;
                break;
            }
        } else {
            break;
        }
    }
    
    if (socket_fd_ < 0) {
        if (!beforeConnectCallback.is_null()) beforeConnectCallback.Run(false);
        if (!callback.is_null()) callback.Run(ERR_ADDRESS_INVALID);
        return socket_fd_;
    }
    
    SetNonBlocking(socket_fd_);
    SetTCPNoDelay(socket_fd_, true);
    SetTCPKeepAlive(socket_fd_, true, 900);
    SetTCPNoSigPipe(socket_fd_);
    
    if (!beforeConnectCallback.is_null()) beforeConnectCallback.Run(true);
    int rv = ::connect(socket_fd_, (sockaddr *)hostaddr_.data(), (socklen_t)hostaddr_.length());
    int ret = rv == 0 ? OK : MapConnectError(errno);
    
    if (ret != ERR_IO_PENDING) {
        callback.Run(ret);
        return ret;
    }
    
    if (!base::MessageLoopForIO::current()->WatchFileDescriptor(
            socket_fd_, true, base::MessageLoopForIO::WATCH_WRITE,
            &write_socket_watcher_, this)) {
        PLOG(ERROR) << "WatchFileDescriptor failed on connect, errno " << errno;
        return errno;
    }
    return ret;
}

int AsyncSocket::resolveHostBeforeConnect() {
    bool fallback_to_default = false;
    if (dns_resolver_callback_.is_null()) {
        fallback_to_default = true;
        // pass through to default route
    } else if (is_dns_resolver_callback_sync_) {
        std::vector<sockaddr_data> results = dns_resolver_callback_.Run(hostname_, port_);
        if (results.size() == 1 && results[0] == UseDefaultResolver) {
            fallback_to_default = true;
            // pass through to default route
        } else if (!results.empty()) {
            sockaddrs_ = results;
            hostname_ = "";
            return 0;
        } else {
            connect_state_ = CONNECT_STATE_NONE;
            write_callback_.Run(ERR_NAME_RESOLUTION_FAILED);
            return ERR_NAME_NOT_RESOLVED;
        }
    }
    // Default route
    auto current_loop = base::MessageLoopProxy::current();
    std::string hostname = hostname_;
    uint16_t port = port_;
    dns_thread_.reset(new base::Thread("AsyncSocketDNSThread"));
    dns_thread_->Start();
    dns_thread_->message_loop()->PostTask(FROM_HERE, base::BindLambda([=](base::WeakPtr<AsyncSocket> weak_this){
        auto callback = fallback_to_default ? DefaultDnsResolver() : dns_resolver_callback_;
        std::vector<sockaddr_data> results = callback.Run(hostname, port);
        if (!fallback_to_default && results.size() == 1 && results[0] == UseDefaultResolver) {
            results = DefaultDnsResolver().Run(hostname, port);
        }
        if (!results.empty()) {
            current_loop->PostTask(FROM_HERE, base::BindLambda(weak_this, [=](){
                dns_thread_->StopSoon();
                dns_thread_.reset();
                sockaddrs_ = results;
                hostname_ = "";
                connect_state_ = CONNECT_STATE_NONE;
                connect(write_callback_, before_connect_callback_);
            }));
        } else {
            current_loop->PostTask(FROM_HERE, base::BindLambda(weak_this, [=](){
                dns_thread_->StopSoon();
                dns_thread_.reset();
                connect_state_ = CONNECT_STATE_NONE;
                if (!before_connect_callback_.is_null()) {
                    before_connect_callback_.Run(true);
                    before_connect_callback_.Reset();
                }
                write_callback_.Run(ERR_NAME_RESOLUTION_FAILED);
            }));
        }
    }, weak_factory_.GetWeakPtr()));
    current_loop->PostDelayedTask(FROM_HERE, base::BindLambda([=](base::WeakPtr<AsyncSocket> weak_this){
        if (weak_this && dns_thread_) {
            if (!before_connect_callback_.is_null()) {
                before_connect_callback_.Run(true);
                before_connect_callback_.Reset();
            }
        }
    }, weak_factory_.GetWeakPtr()), base::TimeDelta::FromSeconds(5));
    return ERR_IO_PENDING;
}

int AsyncSocket::read(IOBuffer* buf, size_t buf_len, const CompletionCallback& callback) {
    DCHECK(thread_checker_.CalledOnValidThread());
    CHECK(read_callback_.is_null());
    // Synchronous operation not supported
    DCHECK(!callback.is_null());
    DCHECK_LT(0, buf_len);
  
    if (connect_state_ != CONNECT_STATE_CONNECT_COMPLETE) {
        callback.Run(ERR_SOCKET_NOT_CONNECTED);
        return 0;
    }
    
    ssize_t rv = ::read(socket_fd_, buf->data(), buf_len);
    int ret = rv >= 0 ? (int)rv : MapSystemError(errno);
    if (ret != 0 && ret != ERR_IO_PENDING) {
        callback.Run(ret);
        return ret;
    }
    
    if (!base::MessageLoopForIO::current()->WatchFileDescriptor(
            socket_fd_, true, base::MessageLoopForIO::WATCH_READ,
            &read_socket_watcher_, this)) {
        PLOG(ERROR) << "WatchFileDescriptor failed on read, errno " << errno;
        return errno;
    }
    
    read_buf_ = buf;
    read_buf_len_ = buf_len;
    read_callback_ = callback;
    return ERR_IO_PENDING;
}

int AsyncSocket::write(IOBuffer* buf, size_t buf_len, const CompletionCallback& callback) {
    DCHECK(thread_checker_.CalledOnValidThread());
    DCHECK(connect_state_ == CONNECT_STATE_CONNECT_COMPLETE);
    CHECK(write_callback_.is_null());
    // Synchronous operation not supported
    DCHECK(!callback.is_null());
    DCHECK_LT(0, buf_len);
    
    ssize_t rv = WriteWrapper(socket_fd_, buf->data(), buf_len);
    int ret = rv >= 0 ? (int)rv : MapSystemError(errno);
    if (ret != 0 && ret != ERR_IO_PENDING) {
        callback.Run(ret);
        return ret;
    }
    
    if (!base::MessageLoopForIO::current()->WatchFileDescriptor(
        socket_fd_, true, base::MessageLoopForIO::WATCH_WRITE,
        &write_socket_watcher_, this)) {
        PLOG(ERROR) << "WatchFileDescriptor failed on write, errno " << errno;
        return errno;
    }
    
    write_buf_ = buf;
    write_buf_len_ = buf_len;
    write_callback_ = callback;
    return ERR_IO_PENDING;
}

void AsyncSocket::disconnect() {
    DCHECK(thread_checker_.CalledOnValidThread());
    
    stopWatchingAndCleanUp();
    close(socket_fd_);
    socket_fd_ = -1;
    connect_state_ = CONNECT_STATE_NONE;
}

bool AsyncSocket::isConnected() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    
    if (connect_state_ != CONNECT_STATE_CONNECT_COMPLETE)
        return false;
    
    // Checks if connection is alive.
    char c;
    ssize_t rv = recv(socket_fd_, &c, 1, MSG_PEEK);
    if (rv == 0)
        return false;
    if (rv == -1 && MapSystemError(errno) != ERR_IO_PENDING)
        return false;
    
    return true;
}

sockaddr_data AsyncSocket::currentSockaddr() const {
    return hostaddr_;
}

void AsyncSocket::OnFileCanReadWithoutBlocking(int fd) {
    DCHECK(!read_callback_.is_null());
    
    ssize_t rv = ::read(socket_fd_, read_buf_->data(), read_buf_len_);
    int ret = rv >= 0 ? (int)rv : MapSystemError(errno);
    if (ret == ERR_IO_PENDING)
        return;
    
    bool ok = read_socket_watcher_.StopWatchingFileDescriptor();
    DCHECK(ok);
    read_buf_ = NULL;
    read_buf_len_ = 0;
    base::ResetAndReturn(&read_callback_).Run(ret);
}

void AsyncSocket::OnFileCanWriteWithoutBlocking(int fd) {
    DCHECK(!write_callback_.is_null());
    
    if (connect_state_ == CONNECT_STATE_CONNECT) {
        // Get the error that connect() completed with.
        int os_error = 0;
        socklen_t len = sizeof(os_error);
        if (getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &os_error, &len) == 0) {
            // TCPSocketLibevent expects errno to be set.
            errno = os_error;
        }
        
        int ret = MapConnectError(errno);
        if (ret == ERR_IO_PENDING)
            return;

        bool ok = write_socket_watcher_.StopWatchingFileDescriptor();
        DCHECK(ok);
        if (ret == OK) {
            connect_state_ = CONNECT_STATE_CONNECT_COMPLETE;
        } else {
            connect_state_ = CONNECT_STATE_NONE;
        }
        base::ResetAndReturn(&write_callback_).Run(ret);
    } else {
        ssize_t rv = WriteWrapper(socket_fd_, write_buf_->data(), write_buf_len_);
        int ret = rv >= 0 ? (int)rv : MapSystemError(errno);
        if (ret == ERR_IO_PENDING)
            return;
        
        bool ok = write_socket_watcher_.StopWatchingFileDescriptor();
        DCHECK(ok);
        write_buf_ = NULL;
        write_buf_len_ = 0;
        base::ResetAndReturn(&write_callback_).Run(ret);
    }
}

void AsyncSocket::stopWatchingAndCleanUp() {
    bool ok = read_socket_watcher_.StopWatchingFileDescriptor();
    DCHECK(ok);
    ok = write_socket_watcher_.StopWatchingFileDescriptor();
    DCHECK(ok);
    
    if (!read_callback_.is_null()) {
        read_buf_ = NULL;
        read_buf_len_ = 0;
        read_callback_.Reset();
    }
    
    if (!write_callback_.is_null()) {
        write_buf_ = NULL;
        write_buf_len_ = 0;
        write_callback_.Reset();
    }
}

DnsResolverCallback AsyncSocket::DefaultDnsResolver() {
    return base::BindLambda([=](const std::string& hostname, uint16_t port) -> std::vector<std::string>{
        struct addrinfo hints, *servinfo, *aip;
        int rv;
        std::vector<sockaddr_data> results;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        std::string porthint = base::UintToString(port);
        LOG(WARNING) << "AsyncSocket DefaultDnsResolver getaddrinfo begins";
        if ((rv = getaddrinfo(hostname.c_str(), porthint.c_str(), &hints, &servinfo)) == 0) {
            std::string iplist;
            for (aip = servinfo; aip; aip = aip->ai_next) {
                sockaddr_data hostaddr((const char*)aip->ai_addr, aip->ai_addrlen);
                results.push_back(hostaddr);
                if (!iplist.empty()) iplist = iplist.append("; ");
                iplist = iplist.append(Sockaddr2IP(aip->ai_addr));
            }
            LOG(WARNING) << "AsyncSocket DefaultDnsResolver getaddrinfo returns " << results.size() << " results (" << iplist << ")";
            freeaddrinfo(servinfo);
            return results;
        } else {
            LOG(ERROR) << "AsyncSocket DefaultDnsResolver getaddrinfo fail: " << gai_strerror(rv);
        }
        return results;
    });
}

std::string AsyncSocket::IPV4toSockaddr(const std::string& ipv4string, uint16_t port) {
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET; /* 主机字节序 */
    my_addr.sin_port = htons(port); /* short, 网络字节序 */
    my_addr.sin_addr.s_addr = inet_addr(ipv4string.c_str());
    return std::string((const char*)&my_addr, sizeof(my_addr));
}

std::string AsyncSocket::IPV6toSockaddr(const std::string& ipv6string, uint16_t port) {
    struct sockaddr_in6 my_addr;
    my_addr.sin6_family = AF_INET6; /* 主机字节序 */
    my_addr.sin6_port = htons(port); /* short, 网络字节序 */
    inet_pton(AF_INET6, ipv6string.c_str(), &my_addr.sin6_addr);
    return std::string((const char*)&my_addr, sizeof(my_addr));
}

std::string AsyncSocket::Sockaddr2IP(const sockaddr* sockaddr) {
    char buf[INET6_ADDRSTRLEN];
    if (sockaddr->sa_family == AF_INET) {
        const sockaddr_in* sockaddr2 = (const sockaddr_in*)sockaddr;
        if (inet_ntop(sockaddr2->sin_family, &sockaddr2->sin_addr, buf, INET6_ADDRSTRLEN)) {
            return buf;
        }
    } else if (sockaddr->sa_family == AF_INET6) {
        const sockaddr_in6* sockaddr2 = (const sockaddr_in6*)sockaddr;
        if (inet_ntop(sockaddr2->sin6_family, &sockaddr2->sin6_addr, buf, INET6_ADDRSTRLEN)) {
            return buf;
        }
    }
    return "";
}
