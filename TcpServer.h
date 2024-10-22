#pragma once
#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include <functional>
#include "EventLoopThreadPool.h"
#include "Callback.h"
#include <unordered_map>
#include <atomic>
#include "TcpConnection.h"


namespace WYXB {
/*
 * 用户使用muduo网络库编程
 * 
 */
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr,const std::string& nameArgs, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback& cb){ threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

    // 设置subloop数量
    void setThreadNum(int numThreads);

    // 开启服务器监听
    void start();
private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string name_;
    const std::string ipPort_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_; // 有新连接的回调
    MessageCallback messageCallback_; // 有数据到达的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调

    ThreadInitCallback threadInitCallback_; // 线程初始化回调
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_; // 保存所有连接
};







}