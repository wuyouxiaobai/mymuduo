#pragma once

#include "noncopyable.h"
#include <functional>
#include "Timestamp.h"
#include <memory>

namespace WYXB
{

class EventLoop;

/*
 * Channel类封装了socketfd和其感兴趣的event，如EPOLLIN和EPOLLOUT
 * 还绑定了poller返回的具体事件
 * 用来处理事件和fd
*/    
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;
    
    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到通知后处理事件
    void handleEvent(Timestamp receiveTime);
    
    // 设置回调函数
    void setReadCallback(ReadEventCallback cb){ readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb){writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb){closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb){errorCallback_ = std::move(cb); }
    
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt){ revents_ = revt; }

    

    // 设置fd相应的事件状态,update()将fd和event类型添加
    void enableReading() {events_ |= kReadEvent; update();}
    void disableReading() {events_ &= ~kReadEvent; update();}
    void enableWriting() {events_ |= kWriteEvent; update();}
    void disableWriting() {events_ &= ~kWriteEvent; update();}
    void disableAll() {events_ = kNoneEvent; update();}

    // 返回fd当前事件状态
    bool isNoneEvent() const {return events_ == kNoneEvent;}
    bool isWriting() const {return events_ & kWriteEvent;}
    bool isReading() const {return events_ & kReadEvent;}

    int index() {return index_;}
    void set_index(int idx) {index_ = idx;}

    // one loop per thread
    // 获得Channel所属的loop
    EventLoop* ownerLoop() {return loop_;}
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);
    

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    
    EventLoop* loop_; // 事件循环
    const int fd_; // 监听的对象
    int events_; // 注册fd感兴趣的事件
    int revents_; // poller返回的具体发生的事件
    int index_; 
    
    std::weak_ptr<void> tie_;
    bool tied_;

    // 通过fd对应的调用事件，调用对应的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
    
};



}




