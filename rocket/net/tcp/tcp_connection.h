#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H
#include "net_addr.h"
#include "tcp_buffer.h"
#include "../io_thread.h"
#include "../eventloop.h"
#include <memory>// use shared_ptr
#include <map>  // use map to store read callback functions
#include <queue> // use queue to store write callback functions
#include <utility> // use pair
#include "../codec/abstract_protocol.h"
#include "../codec/abstract_codec.h"
#include "../rpc/rpc_dispatcher.h"// use rpc_dispatcher to do the actual rpc call

namespace rocket{
// 因为RpcDispatcher类中也引用了tcpconnection类，因此造成了循环引用的问题！
// TODO: 循环引用问题需要 使用 类的前置声明来解决！
class RpcDispatcher;
/*
    class TcpConnection
    成员：
    m_local_addr：本地服务器(客户端)的地址
    m_peer_addr：对端客户端(服务端)的地址

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
enum TcpConnectionType{// review tcp的3挥4握！很容易忘记了！
    TcpConnectionByServer = 1, // 作为服务器端使用，代表跟对端客户端的连接
    TcpConnectionByClient = 2  // 作为客户端使用，  代表跟对端服务端的连接
};
class TcpConnection{
public:
enum TcpConnState{// review tcp的3挥4握！很容易忘记了！
    NotConnected = 1,// 处于未正常连接 状态
    Connected = 2,// 处于正常连接 状态
    HalfClosing = 3,// 处于半连接 状态
    Closed = 4,// 处于连接已经被关闭 状态
};
private:
    // IO_Thread* m_io_thread {nullptr}; // 代表 持有该tcp连接的IO线程（当前到底是那个IO线程需要监听fd事件）
    EventLoop* m_event_loop {nullptr};
    std::shared_ptr<FdEvent> m_fd_event {nullptr}; // 当前tcpConncetion也是一个fd对象

    TcpBuffer::s_ptr m_in_buffer {nullptr};  // 接收缓冲区
    TcpBuffer::s_ptr m_out_buffer {nullptr}; // 发送缓冲区

    NetAddrBase::s_ptr m_local_addr {nullptr};
    NetAddrBase::s_ptr m_peer_addr {nullptr};

    TcpConnState m_state;// 代表 当前tcp连接的状态
    int m_cfd {0};// 代表 当前tcp连接的客户端用于通信的fd
    TcpConnectionType m_connection_type {TcpConnectionByServer};
    AbstractCodec::s_ptr m_codec;// RPC-Codec编解码器对象
    // m_write_dones: write callback functions
    // <key, val> == <AbstractProtocol, write_callback>
    // std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;
    // <key, val> == <req_id, std::function<void(AbstractProtocol::s_ptr)>>
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
    
    // std::shared_ptr<RpcDispatcher> m_rpc_dispatcher;// 仅仅提供给server端使用dispatcher来doRPC调用的事情
    
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
    TcpConnection(EventLoop* eventloop, int cfd, size_t buffer_size, const NetAddrBase::s_ptr& local_addr
        , const NetAddrBase::s_ptr& peer_addr, const TcpConnectionType& type = TcpConnectionByServer);
    // TcpConnection的3个主要方法： onRead, execute, onWrite

    void onRead();// 命名的时候前面加个on代表的是该方法是回调函数的意思
    void execute();
    void onWrite();// 命名的时候前面加个on代表的是该方法是回调函数的意思
    // 启动监听可写事件
    void listenWrite();
    // 启动监听可读事件
    void listenRead();
    inline NetAddrBase::s_ptr getLocalAddr() const { return m_local_addr; }
    inline NetAddrBase::s_ptr getPeerAddr() const { return m_peer_addr; }
    inline void setState(const TcpConnection::TcpConnState& state) { m_state = state; }
    inline void setTcpConnectionType(const TcpConnectionType& type) { m_connection_type = type; }
    inline TcpConnection::TcpConnState getState()const { return m_state; }
    void pushSendMsg(AbstractProtocol::s_ptr msg
    , const std::function<void(AbstractProtocol::s_ptr)>& callback);// push 要发送的Msgs对象
    void pushReadMsg(std::string req_id, const std::function<void(AbstractProtocol::s_ptr)>& callback);// push 要读取的Msgs对象
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

#endif // ROCKET_NET_TCP_TCP_CONNECTION_H