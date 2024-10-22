#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include <unistd.h>
#include <cstring>


namespace WYXB {

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop) // 创建epoll_event(epoll_create() 创建epoll句柄)
    : Poller(loop),
      epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if(epollFd_ < 0)
    {
        LOG_FATAL("create epoll fd failed:%d \n", errno);
    }

}

EpollPoller::~EpollPoller()
{
    ::close(epollFd_);
}

Timestamp EpollPoller::poll(int timeout, ChannelList* activeChannels)
{
    LOG_INFO("func %s, fd total num = %ld", __func__, channels_.size());
    int numEvents = ::epoll_wait(epollFd_, &*events_.begin(), static_cast<int>(events_.size()), timeout);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_INFO("epoll_wait return %d events", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == static_cast<int>(events_.size()))
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_INFO("epoll_wait return 0 events");
    }
    else
    {
        if(savedErrno!= EINTR)
        {
            errno = savedErrno;
            LOG_ERROR("epoll_wait error:%d\n", savedErrno);
        }
    }
    return now;
}
void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    LOG_INFO("func %s,fd = %d, events = %d, index = %d", __func__, channel->fd(), channel->events(), index);
    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);// 将事件注册到epoll中

    }
    else// 已经在epoll中注册过了
    {
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }

    }
}
void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for(int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }


}
// 更新channel
void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    event.data.fd = fd;
    if(::epoll_ctl(epollFd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del fd:%d failed:%d\n", fd, errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod fd:%d failed:%d\n", fd, errno);
        }
    }

}

}