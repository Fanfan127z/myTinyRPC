#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H
#include "net_addr.h"
#include "../fdevent.h"
#include "../eventloop.h"
#include "tcp_connection.h"// use tcp-connection object to 收read/发write data
#include "../codec/abstract_protocol.h"
#include "../timer_event.h"// use 定时器任务事件
#include <functional>
#include <string>
#include <memory>
namespace rocket{

class TcpClient{
public:
    typedef std::shared_ptr<TcpClient> s_ptr;
private:
    NetAddrBase::s_ptr m_local_addr;// 客户端网络地址
    NetAddrBase::s_ptr m_peer_addr;// 对端的网络地址(即当前客户端要connect的服务端的网络地址)
    EventLoop* m_event_loop {nullptr};// 成员变量如果是裸指针，则必须要指向nullptr，否则就会是指向一个随机的值
    int m_fd {-1};
    FdEvent* m_fd_event {nullptr};
    TcpConnection::s_ptr m_connection;
    int m_connect_error_code {0};// 客户端连接的错误代码
    std::string m_connect_error_info;// 客户端连接的错误信息
public:
    TcpClient(const NetAddrBase::s_ptr& peer_addr);
    // 注意，我们所有的连接或读写 connect/write/read 都是 异步 执行的！
    // 如果connect连接 完成(不论是连接成功还是失败，都会执行回调函数done() )
    void Connect(std::function<void()> done);// done是回调函数
    // 异步地 发送 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
    // 如果write发送成功，会调用 done函数，函数的入参就是message对象
    void Write(AbstractProtocol::s_ptr msg, std::function<void(AbstractProtocol::s_ptr)> done);
    // 异步地 读取 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
    // 如果read读取成功，会调用 done函数，函数的入参就是message对象
    void Read(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);

    void Stop();// 结束客户端的loop死循环

    void InitLocalAddr(); // 初始化本地客户端的网络地址

    void AddTimerEvent(TimerEvent::s_ptr timerEvent); // 添加监听定时器任务

    // void CancleTimerEvent(TimerEvent::s_ptr timerEvent); // 取消监听定时器任务
    ~TcpClient();
public:
    inline int GetErrorCode() const { return m_connect_error_code; }
    inline std::string GetErrorInfo() const { return m_connect_error_info; }
    inline NetAddrBase::s_ptr GetPeerAddr() const { return m_peer_addr;}
    inline NetAddrBase::s_ptr GetLocalAddr() const { return m_local_addr;}
};
    
}// rocket

#endif // ROCKET_NET_TCP_TCP_CLIENT_H