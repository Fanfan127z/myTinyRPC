#include "tcp_connection.h"
#include "../fd_event_group.h"
#include "../../common/log.h"
#include <unistd.h> // use read()
namespace rocket{

TcpConnection::TcpConnection(IO_Thread* io_thread, int connectfd
        , size_t buffer_size,const NetAddrBase::s_ptr& peer_addr)
        :m_io_thread(io_thread)
        {
        // 创建接收和发送缓冲区
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_peer_addr = peer_addr;
        // 拿到可用的fdevent
        m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(connectfd);
        /* 监听套接字的可读事件
                注：std::bind(&TcpConnection::read, this)中传入this指针表示只调用属于该对象的TcpConnection::read成员方法！
                当connectfd发生可读时间时，就会去执行read方法
        */
        m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::read, this));
        m_local_addr = std::make_shared<IPv4NetAddr>();
        m_state = NotConnected;
}
void TcpConnection::read(){
    // 1.从 socket缓冲区中，调用系统read函数 读取字节流进入in_buffer中
    if(m_state != Connected){
        INFOLOG("client has already disconnected, client addr[%s], client connectfd[%d]",
                m_peer_addr->toString().c_str(), m_fd_event->getFd());
        return;
    }
    // 2.一直循环读取，只要没有读取完毕就继续读
    bool has_read_all = false;
    while(!has_read_all){
        // ...
    }
}
void TcpConnection::execute(){
    
}
void TcpConnection::write(){
    
}
TcpConnection::~TcpConnection(){
    
}


}// rocket