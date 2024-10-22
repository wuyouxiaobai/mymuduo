#include "Poller.h"
#include "Channel.h"
#include <stdlib.h>
#include "EpollPoller.h" 


namespace WYXB{

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_EPOLL"))
    {
        return nullptr;// 生成epoll实例
    }
    else
    {
        return new EpollPoller(loop);// 生成poll实例
    }

}

}