#include "fd_event_group.h"
#include "../common/log.h"

namespace rocket{
static std::shared_ptr<FdEventGroup> g_fd_event_group = nullptr;
FdEventGroup::FdEventGroup(size_t size):m_size(size){
    m_fd_group.resize(m_size);
    for(int i = 0;i < (int)m_size; ++i){
        m_fd_group[i] = std::make_shared<FdEvent>(i);// 用i作为fdevent对象的fd值！
    }
}

std::shared_ptr<FdEvent> FdEventGroup::getFdEvent(int fd){// 拿到关联fd的FdEvent对象
    ScopeMutex<Mutex> lock(m_mutex);// 上锁，防止多个线程操作同一个对象fdeventgroup
    if(fd < (int)m_size){
        return m_fd_group[fd];
    }
    // 此时fd不存在就自动扩容！
    int new_size = (int)(fd * 1.5);// 按照1.5倍扩容fdevent数组fd_event_group
    m_fd_group.resize(new_size);// new_size此时肯定比原来的old m_size大，所以可以直接.resize()
    for(int i = (int)m_size;i < new_size;++i){
        m_fd_group[i] = std::make_shared<FdEvent>(i);
    }
    m_size = new_size;
    return m_fd_group[fd];
}
// FdEventGroup是被用作一个全局对象
std::shared_ptr<FdEventGroup> getFdEventGroup(){
    if(!g_fd_event_group){
        g_fd_event_group = std::make_shared<FdEventGroup>(128);// 这里写定了是128,后面会改成根据配置文件来修改的
    }
    return g_fd_event_group;
}
FdEventGroup::~FdEventGroup(){

}

}// rocket