#pragma once

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include "Timestamp.h"

namespace WYXB {

/*
 * epoll的实现
 * epoll_create() 创建epoll句柄
 * epoll_ctl() 控制注册事件
 * epoll_wait() 等待事件发生
 */

class EpollPoller : public Poller
{
public:

    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeout, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:

    static const int kInitEventListSize = 16;
    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    // 更新channel
    void update(int operation, Channel* channel);
    using EventList = std::vector<epoll_event>;
    int epollFd_;
    EventList events_;


};

}