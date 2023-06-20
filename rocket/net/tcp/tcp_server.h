#ifndef ROCKET_NET_TCP_TCP_SERVER_H
#define ROCKET_NET_TCP_TCP_SERVER_H

#include "tcp_accepter.h"
#include "net_addr.h"
#include "../eventloop.h"
#include "../io_thread_group.h"
#include "../fdevent.h"
#include <functional>
namespace rocket{
/*
    TcpServer是一个单例对象，只能在主线程main中构建

    mainReactor由主线程运行，其作用如下：通过epoll监听listenfd的可读事件，当可读事件发生之后，调用accept函数
    获取connectfd，然后随机取出一个subReactor，将conncetfd的读写事件注册到这个subReactor的epoll上即可。
    mainReactor只负责建立连接事件，不进行业务处理，也不关心已连接套接字的IO事件。
    subReactor通常有多个，没有给subReactor由一个线程来运行，其注册conncectfd的读写事件，当发生IO事件后
    需要进行业务处理。
*/
class TcpServer{
private:
    NetAddrBase::s_ptr m_local_netaddr {nullptr};// 本地监听地址

    TcpAccepter::s_ptr m_accepter {nullptr};

    EventLoop* m_main_eventloop {nullptr};// mainReactor的loop

    IO_Thread_Group* m_io_thread_group {nullptr};// subReactor组

    FdEvent* m_listen_fd_event;// 监听fd的事件

    int m_client_connect_counts {0};// 客户端连接的总数量
public:
    TcpServer(const NetAddrBase::s_ptr& local_netaddr);
    void start();// 开启IO_Threads的loop以及主线程（即mainReactor）的loop循环
    ~TcpServer();
private:
    void init();
    //  每当有新的客户端连接到来时，需要执行
    void onAccept();
};


}// rocket

#endif // ROCKET_NET_TCP_TCP_SERVER_H