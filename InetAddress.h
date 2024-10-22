#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace WYXB
{
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in addr) : addr_(addr){}

    std::string toIpPort() const;
    std::string toIp() const;
    uint16_t toPort() const;
    const sockaddr_in* getSockAddrInet() const {return &addr_;}
    void setSockAddrInet(const sockaddr_in& addr) {addr_ = addr;}
private:
    sockaddr_in addr_;
};




    
}


