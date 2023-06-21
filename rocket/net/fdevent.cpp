#include "fdevent.h"  
#include <unistd.h>
#include <fcntl.h>
#include <string.h>         // use memset
#include "../common/log.h"  // 引用日志模块


namespace rocket {
FdEvent::FdEvent(){
    memset(&m_listen_event, 0 ,sizeof(m_listen_event));
    // 初始化m_listen_events值,就算用不到也需要你初始化，不初始化就算undefined behavior(Effective C++中提示我要这么干的！)
    // 读写回调函数都先初始化为 空指针
    m_read_callback = nullptr;
    m_write_callback = nullptr;
}
FdEvent::FdEvent(int fd):m_fd(fd){
    memset(&m_listen_event, 0 ,sizeof(m_listen_event));// 初始化m_listen_event值, Effective C++中提示我要这么干的！
    m_read_callback = nullptr;
    m_write_callback = nullptr;
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
    // 监听 读事件
    if(event_type == TriggerEvent::IN_EVENT){   // if不知道这里为啥这么封装的话，可以查看我自己的test/test_epoll_server.cpp这个源文件！
        m_listen_event.events |= EPOLLIN;// 让后续的epoll_wait监听（或者说是）检测对应的m_fd的读缓冲区是否有数据，即检测是否有 读事件触发
        m_read_callback = callback;
    } else {// 监听 写事件
        m_listen_event.events |= EPOLLOUT;// 让后续的epoll_wait监听（或者说是）检测对应的m_fd的写缓冲区是否有数据，即检测是否有 写事件触发
        m_listen_event.data.fd = getFd();
        m_write_callback = callback;
    }
    m_listen_event.data.ptr = this;
}
// set该fd是非阻塞的
void FdEvent::setNonBlock(){
    // 设置fd为非阻塞，
    // 讲文件描述符fd设置为非阻塞的，因为我们的主从reactor模型是不允许阻塞读/写的，读写都是异步的
    int flag = fcntl(m_fd, F_GETFL, 0);// 得到文件描述符的属性
    if(flag & O_NONBLOCK){
        return;// if判断出此时的fd已经是非阻塞的，就直接return
    }
    // 否则，直接set非阻塞
    flag |= O_NONBLOCK;
    fcntl(m_fd, F_SETFL, flag);
}   
FdEvent::~FdEvent(){
    
} 

} // rocket