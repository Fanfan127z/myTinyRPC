#ifndef ROCKET_NET_FD_EVENT_GROUP_H
#define ROCKET_NET_FD_EVENT_GROUP_H

#include <vector>
#include <memory>
#include "fdevent.h"
#include "../common/mutex.h"// use mutex 因为存在多线程操作fdevent的场景

namespace rocket{

/*
    FdEventGroup是对FdEvent的再一次封装
    进程中，fd的值是每创建一个其数值就递逐个增的，不会重复
    因此直接拿一个数组管理起来，更加方便
*/
class FdEventGroup{
private:
    size_t m_size {0};// FdEvent数组的大小
    std::vector<std::shared_ptr<FdEvent>> m_fd_group;// 管理FdEvent的数组
    Mutex m_mutex;
public:
    FdEventGroup(size_t size);
    std::shared_ptr<FdEvent> getFdEvent(int fd);// 拿到关联fd的FdEvent对象
    ~FdEventGroup();
public:
    static std::shared_ptr<FdEventGroup> getFdEventGroup();// FdEventGroup是被用作一个全局对象
};

}// rocket
#endif // ROCKET_NET_FD_EVENT_GROUP_H