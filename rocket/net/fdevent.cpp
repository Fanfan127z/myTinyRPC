#include "fdevent.h"  
#include <string.h>  
#include "../common/log.h"  // 引用日志模块


namespace rocket {

FdEvent::FdEvent(int fd):m_fd(fd){
    memset(&m_listen_events, 0 ,sizeof(m_listen_events));// 初始化m_listen_events值
}
std::function<void()> FdEvent::handler(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        // 返回 读回调函数
        return m_read_callback;
    } else {
        // 返回 写回调函数
        return m_write_callback;
    }
}
void FdEvent::listen(TriggerEvent event_type, const std::function<void()>& callback){
    if(event_type == TriggerEvent::IN_EVENT){   // if不知道这里为啥这么封装的话，可以查看我自己的test/test_epoll_server.cpp这个源文件！
        m_listen_events.events |= EPOLLIN;// 让后续的epoll_wait监听（或者说是）检测对应的m_fd的读缓冲区是否有数据，即检测是否有 读事件触发
        m_read_callback = callback;
    } else {
        m_listen_events.events |= EPOLLOUT;// 让后续的epoll_wait监听（或者说是）检测对应的m_fd的写缓冲区是否有数据，即检测是否有 写事件触发
        m_listen_events.data.fd = getFd();
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;
}
FdEvent::~FdEvent(){
    
} 

} // rocket