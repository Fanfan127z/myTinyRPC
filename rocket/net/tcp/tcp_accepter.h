#ifndef ROCKET_NET_TCP_TCP_ACCEPTER_H
#define ROCKET_NET_TCP_TCP_ACCEPTER_H
#include "net_addr.h"

namespace rocket{
// TcpAccepter的核心功能就是 有监听到来，就要 调用accept()函数接受客户端的连接
class TcpAccepter{
private:
    NetAddrBase::s_ptr m_local_addr;
    // 服务器监听的地址，addr -> ip : port
    IPv4NetAddr m_netAddr;
    // 所监听客户端的socket_addr
    struct sockaddr_in m_client_addr;
    socklen_t m_client_addr_len {0};
    int m_family {-1};// IPv4(AF_INET) or IPv6(AF_INET6)
    int m_listenfd {-1};// 监听套接字
    int m_connectfd {-1};// 通信套接字
public:
    TcpAccepter(const NetAddrBase::s_ptr& netaddr);
    TcpAccepter(const IPv4NetAddr& netaddr);
    int accept();
    ~TcpAccepter();
};

}// rocket
#endif // ROCKET_NET_TCP_TCP_ACCEPTER_H