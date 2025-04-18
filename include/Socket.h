#pragma once
#include "noncopyable.h"


namespace WYXB
{
class InetAddress;
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int sockfd() const { return sockfd_; }
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setKeepAlive(bool on);
    void setReusePort(bool on);


private:
    const int sockfd_;


};


}