#ifndef ROCKET_NET_TCP_NET_ADDR_H
#define ROCKET_NET_TCP_NET_ADDR_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <memory>

namespace rocket{
class NetAddrBase{
private:

public:
    typedef std::shared_ptr<NetAddrBase> s_ptr;
    NetAddrBase() = default;
    virtual const struct sockaddr* getSocketAddr() = 0;
    virtual socklen_t getSocketLen() = 0;
    virtual int getFamily() = 0;
    virtual const std::string toString() = 0;
    virtual bool checkValid() = 0;// 检测sockaddr的合法性
    virtual ~NetAddrBase() = default;// base class 之析构函数必须是虚的析构函数
};

class IPv4NetAddr : public NetAddrBase{
private:
    std::string m_ip;
    uint16_t m_port {0};
    struct sockaddr_in m_addr;
public:
    IPv4NetAddr() = default;
    IPv4NetAddr(const std::string& ip, uint16_t port);
    IPv4NetAddr(const std::string& addr);
    IPv4NetAddr(const struct sockaddr_in& addr);
    
    ~IPv4NetAddr();

    const struct sockaddr* getSocketAddr() override;
    socklen_t getSocketLen() override;
    int getFamily() override;
    
    const std::string toString() override;
    bool checkValid() override;
private:
    void init();
};
}// rocket
#endif // ROCKET_NET_TCP_NET_ADDR_H