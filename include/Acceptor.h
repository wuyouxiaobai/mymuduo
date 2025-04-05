#pragma once
#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"

namespace WYXB
{

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnecionCallback = std::function<void(int sockfd, const InetAddress& addr)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnecionCallback& cb)
    { newConnectionCallback_ = cb; }

    [[nodiscard]] bool listening() const { return listenning_; }
    void listen();


private:
    void handleRead(); // 处理新连接事件
    EventLoop* loop_; // Accepto使用用户定义的baseloop，也即mainloop
    Socket acceptSocket_; // 监听套接字
    Channel acceptChannel_; // 监听事件通道
    NewConnecionCallback newConnectionCallback_; // 新连接回调函数
    bool listenning_; // 是否正在监听



};



}