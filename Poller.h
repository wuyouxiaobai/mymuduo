#pragma once

#include "noncopyable.h"
#include <vector>
#include <map>
#include "Timestamp.h"


namespace WYXB{


class Channel; 
class EventLoop;

// muduo网络库中多路事件分发器的核心模块io复用模块
class Poller : noncopyable 
{
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0; 
    
    // 判断参数channel是否在当前Poller中
    bool hasChannel(Channel* channel) const ;

    //Eventloop通过通过获得 接口获得默认io复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);


protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* loop_;

};

}
