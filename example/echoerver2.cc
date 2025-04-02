#include "../include/TcpServer.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <atomic>


// test err ?????????????????????


class EchoServer
{
private:
    WYXB::TcpServer server_;
    WYXB::EventLoop* loop_;
    std::atomic<int> clientCount_; // 记录当前连接的客户端数量

    void onConnection(const WYXB::TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            clientCount_++;
            LOG_INFO("Connection UP: %s, Total clients: %d", conn->peer_addr().toIpPort().c_str(), clientCount_.load());
        }
        else
        {
            clientCount_--;
            LOG_INFO("Connection DOWN: %s, Total clients: %d", conn->peer_addr().toIpPort().c_str(), clientCount_.load());
        }
    }

    void onMessage(const WYXB::TcpConnectionPtr& conn, WYXB::Buffer* buf, WYXB::Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        LOG_INFO("Received data from %s: %s", conn->peer_addr().toIpPort().c_str(), msg.c_str());
        conn->send(msg); // 回显消息
        // conn->shutdown(); // 如果需要立即关闭连接，可以取消注释
    }

public:
    EchoServer(WYXB::EventLoop* loop, const WYXB::InetAddress& addr, const std::string& name)
        : server_(loop, addr, name), loop_(loop), clientCount_(0)
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setThreadNum(4); // 设置工作线程数
    }

    ~EchoServer() = default;

    void start()
    {
        LOG_INFO("Starting EchoServer");
        server_.start();
    }
};

// 模拟客户端连接
void simulateClient(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG_ERROR("Failed to create socket");
        return;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        LOG_ERROR("Failed to connect to server");
        close(sockfd);
        return;
    }

    std::string msg = "Hello from client";
    if (write(sockfd, msg.c_str(), msg.size()) < 0)
    {
        LOG_ERROR("Failed to send data");
    }

    char buffer[1024];
    ssize_t n = read(sockfd, buffer, sizeof(buffer));
    if (n > 0)
    {
        buffer[n] = '\0';
        LOG_INFO("Received echo: %s", buffer);
    }

    close(sockfd);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    try
    {
        WYXB::EventLoop loop;
        WYXB::InetAddress addr(port);
        EchoServer server(&loop, addr, "EchoServer");

        server.start();

        // 模拟多个客户端并发连接
        std::vector<std::thread> clients;
        for (int i = 0; i < 10; ++i)
        {
            clients.emplace_back(simulateClient, port);
        }

        for (auto& t : clients)
        {
            t.join();
        }

        loop.loop();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception: %s", e.what());
        return 1;
    }

    return 0;
}