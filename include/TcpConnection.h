#pragma once

#include <memory>
#include <string>
#include <atomic>
#include "InetAddress.h"
#include "Callback.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "noncopyable.h"


namespace WYXB
{



class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr);
    ~TcpConnection();

    EventLoop* getLoop() const{ return loop_; }
    const std::string& name() const{ return name_; }
    const InetAddress& local_addr() const{ return local_addr_; }
    const InetAddress& peer_addr() const{ return peer_addr_; }

    bool connected() const{ return state_ == kConnected; }

    void send(std::string_view buf);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { highWaterMarkCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }


    void connectEstablished();
    void connectDestroyed();


private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting}; 


    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, int len);


    void shutdownInLoop();


    EventLoop* loop_; // 不可能是baseloop，因为这个connection是在subloop里管理的
    std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress local_addr_;
    const InetAddress peer_addr_;

    ConnectionCallback connectionCallback_; // 有新连接和连接断开的回调
    MessageCallback messageCallback_; // 有数据到达的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_; // 消息积压的回调
    CloseCallback closeCallback_; // 连接关闭的回调
    size_t highWaterMark_; // 消息积压的阈值

    Buffer inputBuffer_; // 输入缓冲区
    Buffer outputBuffer_; // 输出缓冲区


    inline static constexpr size_t kDefaultHighWaterMark = 64 * 1024 * 1024;
};



}