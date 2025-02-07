#include "EventLoop.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <unistd.h>
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

namespace WYXB
{

// 防止一个线程创建多个eventloop对象
__thread EventLoop* t_loopInThisThread = nullptr;

// 定义Poller io复用接口超时时间
const int kPollTimeoutMs = 10000;

int createEventfd()
{   
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        LOG_FATAL("eventfd create failed:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false), 
      callingPendinFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p is created in this thread %d", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    // 设置wakeupfd事件类型及发生后回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每个eventloop都监听wakeupchannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();;
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;

}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
    }
}


void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p starts loop", this);
    while(!quit_)
    {
        activeChannels_.clear();
        // 监听clientfd和wakeupfd的事件发生
        pollReturnTime_ = poller_->poll(kPollTimeoutMs, &activeChannels_);
        for(Channel* channel : activeChannels_)
        {
            // Poller监听哪些channel事件发生了，然后上报给eventloop，通知channel处理相应事件
            channel->handleEvent(pollReturnTime_);
        }
        /** 执行当前eventloop事件循环需要处理的所有回调函数
         *  将当前loop需要执行的回调函数放入待执行队列，被wakeup后，执行mainloop注册的函数
         * 
         */
        doPendingFunctors();
      
    }

    LOG_INFO("EventLoop %p stops loop", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }

}

// 在当前loop中执行回调函数
void EventLoop::runInLoop(Functor&& cb)
{
    if(isInLoopThread())
    {
        cb(); // 直接执行
    }
    else
    {
        queueInLoop(std::move(cb)); // 放入待执行队列
    }
}

// 用来唤醒loop所在的线程 向wakeupfd写入数据，wakeupchannel的EPOLLIN读事件发生后，当前线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    
    }
    if(!isInLoopThread() || callingPendinFunctors_)
    {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) const
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() // 执行待执行的回调函数
{
    std::vector<Functor> functors;
    callingPendinFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(Functor& functor : functors)
    {
        functor(); // 执行当前loop需要执行的回调函数
    }
    callingPendinFunctors_ = false;
}


}