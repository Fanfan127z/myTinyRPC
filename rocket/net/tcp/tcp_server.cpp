#include "tcp_server.h"
#include "tcp_connection.h"

namespace rocket{

TcpServer::TcpServer(const NetAddrBase::s_ptr& local_netaddr)
                    :m_local_netaddr(local_netaddr){
    init();
    INFOLOG("RPC TcpServer listen success on [%s]", m_local_netaddr->toString().c_str());
}

// 开启IO_Threads的loop以及主线程（即mainReactor）的loop循环
void TcpServer::start(){
    m_io_thread_group->start();
    // m_io_thread_group->join();
    m_main_eventloop->loop();// 正常情况下main线程一直阻塞，是死循环，永远不结束
}
void TcpServer::init(){
    m_accepter = std::make_shared<TcpAccepter>(m_local_netaddr);
    
    m_main_eventloop = EventLoop::GetCurrentEventLoop();// 主线程的eventloop就是main_eventloop，这就是mainReactor
    // 这里暂时写个2，表示有2个IO_Thread，后面需要revise成配置参数传入的值
    m_io_thread_group = new IO_Thread_Group(2);

    m_listen_fd_event = new FdEvent(m_accepter->getListenfd());// 监听的fdEvent事件
    // 给它加一个监听，注意，这里一定是监听的可读事件，因为每来一个客户端连接write了（无关紧要的东西到RPC-Server）
    // 时，epoll会监听到当前的有可读事件触发！然后来执行对应之callback(回调任务函数)
    // void FdEvent::listen(TriggerEvent event_type, const std::function<void()>& callback)
    m_listen_fd_event->listen(
        FdEvent::TriggerEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_eventloop->addEpollEvent(m_listen_fd_event);
}

//  每当有新的客户端连接到来时，需要执行
void TcpServer::onAccept(){
    std::pair<int, rocket::NetAddrBase::s_ptr> ret_pair = m_accepter->accept();
    m_client_connect_counts++;// 客户端连接数+1
    int connectfd = ret_pair.first;
    /*      
        把connectfd添加到 任意的IO线程中去do事情(TcpConnection类)
        也即client connectfd所触发的IO事件 都会由 新的IO线程 去执行
    */
    NetAddrBase::s_ptr peer_addr = ret_pair.second;
    // TODO: 这里的buffersize为128是随便初始化的，后面需要更改成配置参数的传参！
    IO_Thread* io_thread = m_io_thread_group->getAvailableIO_Thread().get();
    TcpConnection::s_ptr conn = 
        std::make_shared<TcpConnection>(io_thread->getEventLoop(), connectfd, 128, m_local_netaddr, peer_addr);
    // 每accept之后拿到一个tcp连接，就说明该tcp连接已经处于正常情况了！
    conn->setState(TcpConnection::TcpConnState::Connected);
    m_client_connections.insert(conn);
    // conn->execute();
    INFOLOG("TcpServer success get a client connection, cfd = [%d]", connectfd);
}
TcpServer::~TcpServer(){
    if(m_main_eventloop){
        delete m_main_eventloop;
        m_main_eventloop = nullptr;
    }
    if(m_io_thread_group){
        delete m_io_thread_group;
        m_io_thread_group = nullptr;
    }
    if(m_listen_fd_event){
        delete m_listen_fd_event;
        m_listen_fd_event = nullptr;
    }
    m_client_connect_counts = 0;// 客户端连接数 置零
    INFOLOG("~TcpServer()");
}

// 客户端连接RPC-Server时，如果断开连接之后我需要从m_client_connections中删除这个client(如何删除，得学习大佬的代码和思路)
// 否则，会出现一直read/write 回应客户端时的error错误！
void TcpServer::clearClientConnections(){
    
}

}// rocket