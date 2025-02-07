#include "Acceptor.h"
#include "Logger.h"
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "InetAddress.h"
#include "unistd.h"

namespace WYXB
{

static int createNoneBlockSocket()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d Failed in create socket with error: %s", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort)
        : loop_(loop),
          acceptSocket_(createNoneBlockSocket()),
          acceptChannel_(loop, acceptSocket_.sockfd()),
          listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);  // bind socket to listen address
    // TcpServer::start() Acceptor::listen() 有新用户连接执行一个回调（connfd -》 channel -》 subloop）
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));

}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}


// 监听fd有事件发生，即新用户连接，调用当前回调函数
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);  // 轮询找到subloop，并执行回调函数
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d Failed in accept new connection with error: %s", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reach limi", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}


}