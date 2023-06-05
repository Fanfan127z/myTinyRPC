#include "fdevent.h"    
#include "../common/log.h"  // 引用日志模块


namespace rocket {

// FdEvent::FdEvent(){

// }
void FdEvent::handler(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        // 执行 读回调函数
        m_read_callback();
    } else {
        // 执行 写回调函数
        m_write_callback();
    }
}
void FdEvent::listen(TriggerEvent event_type, const std::function<void()>& callback){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    } else {
        m_listen_events.events |= EPOLLOUT;
        m_listen_events.data.fd = getFd();
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;
}
FdEvent::~FdEvent(){
    
} 

} // rocket