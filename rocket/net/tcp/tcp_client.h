#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H
#include "net_addr.h"
#include "../fdevent.h"
#include "../eventloop.h"
#include "tcp_connection.h"// use tcp-connection object to 收read/发write data
#include "../codec/abstract_protocol.h"
#include <functional>
#include <string>
namespace rocket{

class TcpClient{
private:
    NetAddrBase::s_ptr m_local_addr;// 客户端网络地址
    NetAddrBase::s_ptr m_peer_addr;// 对端的网络地址(即当前客户端要connect的网络地址)
    EventLoop* m_event_loop {nullptr};// 成员变量如果是裸指针，则必须要指向nullptr，否则就会是指向一个随机的值
    int m_fd {-1};
    FdEvent* m_fd_event {nullptr};
    TcpConnection::s_ptr m_connection;
public:
    TcpClient(const NetAddrBase::s_ptr& peer_addr);
    // 注意，我们所有的连接或读写 connect/write/read 都是 异步 执行的！
    // 如果connect连接成功，则回调函数done()会被执行！
    void Connect(const std::function<void()>& done);// done是回调函数
    // 异步地 发送 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
    // 如果write发送成功，会调用 done函数，函数的入参就是message对象
    void Write(AbstractProtocol::s_ptr msg, std::function<void(AbstractProtocol::s_ptr)> done);
    // 异步地 读取 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
    // 如果read读取成功，会调用 done函数，函数的入参就是message对象
    void Read(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);

    ~TcpClient();
};
    
}// rocket

#endif // ROCKET_NET_TCP_TCP_CLIENT_H