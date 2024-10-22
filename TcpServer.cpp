#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include <string.h>
#include "InetAddress.h"
#include "TcpConnection.h"


namespace WYXB
{

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d main loop is nullptr!! \n", __FILE__, __FUNCTION__ ,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArgs, Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(nameArgs),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(nullptr),
      messageCallback_(nullptr),
      nextConnId_(1),
      started_(0)
{
    // 当有用户连接时，会执行TcpServer::newConnection()回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));

}

TcpServer::~TcpServer()
{
    for(auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

// 有一个新的客户端连接会调用这个函数
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    // 轮询算法，选择一个subloop，来管理channel
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 通过sockfd获得其绑定的本机ip地址和端口号
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) != 0)
    {
        LOG_ERROR("TcpServer::newConnection [%s] - getsockname error\n", name_.c_str());
    }
    InetAddress localAddr(local);

    // 根据连接成功的sockfd，创建TcpConnection对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    // 下面的回调都是用户设置给TcpServer=》TcpConnection=》Channel=》Poller=》notify() Channel调用回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

}
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));

}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removeConnection [%s] - connection [%s]\n", name_.c_str(), conn->name().c_str());

    size_t n = connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

// 设置subloop数量
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);

}

// 开启服务器监听
void TcpServer::start()
{
    if(started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }

}




}