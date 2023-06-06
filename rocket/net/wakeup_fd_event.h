#ifndef ROCKET_NET_WAKEUP_FD_EVENT_H
#define ROCKET_NET_WAKEUP_FD_EVENT_H
#include "fdevent.h"
namespace rocket {
class WakeUpFdEvent : public FdEvent{
public:
    WakeUpFdEvent(int fd);
    // void init();// 实际上，我们只关心wakeup的可读事件！
    void wakeup();// 触发 读事件
    ~WakeUpFdEvent();
private:

};


}// rocket
#endif // ROCKET_NET_WAKEUP_FD_EVENT_H