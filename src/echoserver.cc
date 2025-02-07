#include "../include/TcpServer.h"
#include <string>


class echoserver
{
private:
    WYXB::TcpServer server_;
    WYXB::EventLoop* loop_;

    void onConnection(const WYXB::TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP : %s", conn->peer_addr().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s", conn->peer_addr().toIpPort().c_str());
        }

    }

    void onMessage(const WYXB::TcpConnectionPtr& conn, WYXB::Buffer* buf, WYXB::Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        LOG_INFO("Receive data : %s", msg.c_str());
        conn->send(msg);
        conn->shutdown();
    }

public:
    echoserver(WYXB::EventLoop* loop, const WYXB::InetAddress &addr, const std::string &name) 
    : server_(loop, addr, name)
    , loop_(loop)
    {
        server_.setConnectionCallback(std::bind(&echoserver::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&echoserver::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
        server_.setThreadNum(3);
    }
    ~echoserver() = default;

    void start()
    {
        server_.start();

    }
};

int main(int argc, char* argv[])
{
    WYXB::EventLoop* loop = new WYXB::EventLoop; 
    WYXB::InetAddress addr(8000);
    echoserver server(loop, addr, "echoserver");
    server.start();
    loop->loop();
}

