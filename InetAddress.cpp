#include "InetAddress.h"
#include <cstring>
#include <iostream>

namespace WYXB{


InetAddress::InetAddress(uint16_t port, std::string ip)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIpPort() const
{
    char ipstr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr_.sin_addr, ipstr, sizeof(ipstr))) {
        return std::string(ipstr) + ":" + std::to_string(ntohs(addr_.sin_port));
    } else {
        return "<invalid address>";
    }
}
std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    // 网络字节序转换成字符串
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
    
}
uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
    
}
    
}

#ifdef INETADDRESS_TEST
int main()
{
    WYXB::InetAddress addr(8080, "127.0.0.1");
    std::cout << addr.toIpPort() << std::endl;
    return 0;
    
}

#endif