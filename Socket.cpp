#include "Socket.h"
#include "unistd.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "Logger.h"
#include <string.h>
#include <netinet/tcp.h>
#include "InetAddress.h"



namespace WYXB
{

Socket::~Socket()
{
    ::close(sockfd_);

}

void Socket::bindAddress(const InetAddress& localaddr)
{
    if(0 != ::bind(sockfd_, (sockaddr*)localaddr.getSockAddrInet(), sizeof(sockaddr_in)))
    {
        LOG_FATAL("Socket::bindAddress failed, error: %s", strerror(errno));
    }
}
void Socket::listen()
{
    if(0 != ::listen(sockfd_, SOMAXCONN))
    {
        LOG_FATAL("Socket::listen failed, error: %s", strerror(errno));
    }

}
int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0)
    {
        LOG_ERROR("Socket::accept failed, error: %s", strerror(errno));
    }
    peeraddr->setSockAddrInet(addr);
    return connfd;



}

void Socket::shutdownWrite()
{
    if(0 != ::shutdown(sockfd_, SHUT_WR))
    {
        LOG_ERROR("Socket::shutdownWrite failed, error: %s", strerror(errno));
    }
}


void Socket::setTcpNoDelay(bool on)
{
    int flag = on ? 1 : 0;
    if(::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0)
    {
        LOG_ERROR("Socket::setTcpNoDelay failed, error: %s", strerror(errno));
    }

}
void Socket::setReuseAddr(bool on)
{
    int flag = on ? 1 : 0;
    if(::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
    {
        LOG_ERROR("Socket::setReuseAddr failed, error: %s", strerror(errno));
    }

}
void Socket::setKeepAlive(bool on)
{
    int flag = on ? 1 : 0;
    if(::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) < 0)
    {
        LOG_ERROR("Socket::setKeepAlive failed, error: %s", strerror(errno));
    }

}
void Socket::setReusePort(bool on)
{
    int flag = on ? 1 : 0;
    if(::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0)
    {
        LOG_ERROR("Socket::setReusePort failed, error: %s", strerror(errno));
    }

}



}