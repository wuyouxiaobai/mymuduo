#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include "Timestamp.h"
#include <memory>
#include <mutex>
#include "CurrentThread.h"


namespace WYXB{

class Channel;
class Poller;

// 时间循环类 主要包含channel和Poller的管理
class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    // 启动事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }
    void runInLoop(Functor&& cb); // 在loop线程中执行回调函数
    void queueInLoop(Functor&& cb); // 将回调函数放入待执行队列

    void wakeup(); // 唤醒loop线程

    void updateChannel(Channel* channel); // 更新channel状态
    void removeChannel(Channel* channel); // 移除channel
    bool hasChannel(Channel* channel) const; // 判断channel是否活跃

    [[nodiscard]] bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); } // 判断当前线程是否是loop线程


private:
    void handleRead(); // wake up
    void doPendingFunctors(); // 执行待执行的回调参数


    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_; 
    std::atomic_bool quit_; // 标识是否退出

    const pid_t threadId_; // 当前loop线程id
    Timestamp pollReturnTime_; // poller返回发生事件的channel的时间
    std::unique_ptr<Poller> poller_; // 事件轮询器
    int wakeupFd_; // 当mainloop获得新用户的channel时，通过该变量唤醒一个subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_; // 唤醒subloop的channel
    ChannelList activeChannels_; // 存放活跃的channel


    std::atomic_bool callingPendinFunctors_;// 标识当前loop是否有需要执行的回调参数
    std::vector<Functor> pendingFunctors_; // 存放待执行的回调参数
    std::mutex mutex_; // 互斥锁


};


}
