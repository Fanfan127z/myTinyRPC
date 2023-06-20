#include "net_addr.h"
#include "../../common/log.h"// use log

#include <string.h>// use memset

namespace rocket{

IPv4NetAddr::IPv4NetAddr(const std::string& ip, uint16_t port):m_ip(ip), m_port(port){
    init();
}
void IPv4NetAddr::init(){
    memset(&m_addr, 0, sizeof(m_addr));// 先给m_addr这个struct sockaddr_in结构体对象初始化置零(空)
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());// 点分10进制的ip地址转换为大端的整形数
    m_addr.sin_port = htons(m_port);// 将一个短整形端口号port从主机字节序 -> 网络字节序
    INFOLOG("success init IPv4NetAddr[%s:%d]", m_ip.c_str(), m_port);
}
IPv4NetAddr::IPv4NetAddr(const std::string& addr){// example: addr = "127.0.0.1:9999"
    size_t idx = addr.find_first_of(":");
    if(idx == addr.npos){
        ERRORLOG("IPv4NetAddr error, [%s] is an invalid IPv4 addr", addr.c_str());
        return;
    }
    std::string ip = addr.substr(0, idx);
    uint16_t port = std::stoi(addr.substr(idx+1, addr.size() - idx - 1));// 这个随便画个图就know了
    m_ip = ip, m_port = port;
    init();
}
IPv4NetAddr::IPv4NetAddr(const struct sockaddr_in& addr):m_addr(addr){
    m_ip = std::string(inet_ntoa(m_addr.sin_addr));
    m_port = ntohs(m_addr.sin_port);
    INFOLOG("success init IPv4NetAddr[%s:%d] in IPv4NetAddr(const struct sockaddr_in& addr)", m_ip.c_str(), m_port);
}

const struct sockaddr* IPv4NetAddr::getSocketAddr(){
    return (const struct sockaddr*)&m_addr;
}

socklen_t IPv4NetAddr::getSocketLen(){
    return (socklen_t)sizeof(m_addr);
}

int IPv4NetAddr::getFamily(){
    return AF_INET;
}

bool IPv4NetAddr::checkValid(){
    if(m_ip.empty()){// IP地址值为空，error！
        return false;
    }
    /*
        资源有限：在TCP/IP协议中，端口号是一个16位的数字，也就是说最多有65536个端口。
        但实际上，1-1024号端口已经被系统预留，只有1025-65535号端口可供应用程序使用。
        如果不允许端口复用，那么同时进行的连接数将受到严重限制。
    */
    if(m_port < 0 || m_port > 65536){
        return false;// 主机端口号理论值是0~65535
    }
    /*  man inet_addr:
        The inet_addr() function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data in network byte order. 
        If the input is invalid, INADDR_NONE  (usually  -1)  is  returned
    */
    if(inet_addr(m_ip.c_str()) == INADDR_NONE){
        return false;
    }
    return true;
}
const std::string IPv4NetAddr::toString(){
    std::string ret = m_ip + ":" + std::to_string(m_port);
    // DEBUGLOG("IPv4NetAddr::toString() == [%s]", ret.c_str());// test codes!
    return ret;// example: addr = "127.0.0.1:9999"
}

IPv4NetAddr::~IPv4NetAddr(){
    
}

}// rocket