#ifndef ROCKET_NET_TCP_CONNECTION_H
#define ROCKET_NET_TCP_CONNECTION_H
#include "net_addr.h"
#include "tcp_buffer.h"
#include "../io_thread.h"
#include <memory>

namespace rocket{
/*
    class TcpConnection
    成员：
    m_local_addr：本地服务器的地址
    m_peer_addr：对端客户端的地址

    一个tcp连接，对应有一个输入一个输出缓冲区

    m_in_buffer：
        服务端调用::read成功之后从socket缓冲区读取到数据，会写入到m_in_buffer后面
        服务端从m_in_buffer前面读取数据，进行解码得到请求。
    m_out_buffer:
        服务端向外发送数据，会将数据编码后写入到m_out_buffer后面
        服务端在fd可写的情况下，调用::write将m_out_buffer里面的数据全部一次性发送出去
    方法：
    onRead:读取客户端发来的数据，组装为 RPC请求
    execute:将 RPC请求作为入参，执行业务逻辑得到RPC响应，再把响应发送回去
    onWrite:将 RPC 响应发送给客户端
*/
class TcpConnection{
public:
enum TcpConnState{// review tcp的3挥4握！很容易忘记了！
    NotConnected = 1,// 处于未正常连接 状态
    Connected = 2,// 处于正常连接 状态
    HalfClosing = 3,// 处于半连接 状态
    Closed = 4,// 处于连接已经被关闭 状态
};
private:
    IO_Thread* m_io_thread {nullptr}; // 代表 持有该tcp连接的IO线程（当前到底是那个IO线程需要监听fd事件）
    std::shared_ptr<FdEvent> m_fd_event {nullptr}; // 当前tcpConncetion也是一个fd对象

    TcpBuffer::s_ptr m_in_buffer {nullptr};  // 接收缓冲区
    TcpBuffer::s_ptr m_out_buffer {nullptr}; // 发送缓冲区

    NetAddrBase::s_ptr m_local_addr {nullptr};
    NetAddrBase::s_ptr m_peer_addr {nullptr};

    TcpConnState m_state;// 代表 当前tcp连接的状态
    int m_cfd {0};// 代表 当前tcp连接的客户端用于通信的fd

public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
    TcpConnection(IO_Thread* io_thread, int cfd, size_t buffer_size
        , const NetAddrBase::s_ptr& peer_addr);
    // TcpConnection的3个main方法： onRead, execute, write
    void onRead();// 命名的时候前面加个on代表的是该方法是回调函数的意思
    void execute();
    void onWrite();// 命名的时候前面加个on代表的是该方法是回调函数的意思

    inline void setState(const TcpConnection::TcpConnState& state){ m_state = state; }
    inline TcpConnection::TcpConnState getState()const{ return m_state; }
    /*
        服务器端主动断开连接
        shutdown()的主要目的：
            正常情况下，服务端是不会主动关闭连接的，主动关闭是为了防止以下情况的发生：
            如果有很多客户端，发起TCP连接之后不做任何事情，长时间不做任何收发数据的IO操作
            这样，TCP连接越多，消耗服务器端的资源就越多
            所以，需要处理这些《无效的》TCP连接，要do这个，就需要服务端主动发起断开连接的操作
            原理上：就是服务端主动触发close动作然后经过四次挥手之后关闭该TCP连接。
    */
    void shutdown();// 处理服务器端主动关闭连接后的清理工作
    ~TcpConnection();
private:
    void clear();// 处理一些（来自客户端client主动）关闭连接后的清理工作
};

}// rocket

#endif // ROCKET_NET_TCP_CONNECTION_H