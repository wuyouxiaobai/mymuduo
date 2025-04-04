#include "Channel.h"
#include "sys/epoll.h"
#include "Logger.h"
#include "EventLoop.h"


namespace WYXB
{



Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
        fd_(fd),
        events_(0),
        revents_(0),
        index_(-1),
        tied_(false)
{}

Channel::~Channel()
{}


void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;

}

void Channel::remove()
{
    loop_->removeChannel(this);
}



void Channel::update()
{
    // 更改fd对应事件的
    loop_->updateChannel(this);
}



void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        if(auto guard = tie_.lock(); guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_DEBUG("channel handle revent:%d\n", revents_);
    if((revents_ & EPOLLHUP) && (revents_ & EPOLLERR))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}



}