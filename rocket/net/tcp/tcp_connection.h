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
    read:读取客户端发来的数据，组装为 RPC请求
    execute:将 RPC请求作为入参，执行业务逻辑得到RPC响应
    write:将 RPC 响应发送给客户端
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
public:
    TcpConnection(IO_Thread* io_thread, int connectfd
        , size_t buffer_size,const NetAddrBase::s_ptr& peer_addr);
    // TcpConnection的3个main方法： read, execute, write
    void read();
    void execute();
    void write();
    ~TcpConnection();
};

}// rocket

#endif // ROCKET_NET_TCP_CONNECTION_H