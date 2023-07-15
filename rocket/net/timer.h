#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H
#include <map> // use multimap
#include <string> // use string
#include "timer_event.h" // use TimerEvent
#include "fdevent.h" // use FdEvent
#include "../common/mutex.h" // use mutex
namespace rocket{

/*
    2.4.2 Timer 定时器
    Timer 是一个 TimerEvent 的集合
    Timer 继承自 FdEvent
    方法：
    addTimerEvent();
    deleteTimerEvent();
    
    当Timer所绑定的fd被epoll_wait检测到有IO事件触发(发生)了之后，就需要执行任务函数
    那么这个任务函数(或者说方法就是onTimer)
    onTimer();

    reserArriveTime();
    multimap<key(arrivetime), TimerEvent> : 按照到达时间arrivetime来存储管理我们的TimerEvent， 因为到达时间arrivetime有可能重复（同一个时间执行多个任务），这是复合逻辑的！so不用map来管理！
*/

class Timer : public FdEvent{
private:
    Mutex m_mutex;
    // : 按照到达时间arrivetime来存储管理我们的TimerEvent， 因为到达时间arrivetime有可能重复（同一个时间执行多个任务），这是复合逻辑的！so不用map来管理！
    std::multimap<int64_t, rocket::TimerEvent::s_ptr> m_pending_events;

public:
    Timer();
    ~Timer();
    
    bool addTimerEvent(const rocket::TimerEvent::s_ptr& event);
    bool deleteTimerEvent(const rocket::TimerEvent::s_ptr& event);
    /*
        当Timer所绑定的fd被epoll_wait检测到有IO事件触发(发生)了之后，就需要执行任务函数
        那么这个任务函数(或者说方法就是onTimer)
    */
    /*
        onTimer是该类 中 最关键的函数！
        作用是：在我们触发的可读事件上需要调用的回调函数
    */
    void onTimer();// 发生了IO事件之后，eventloop 会执行这个回调函数
private:
    void resetArriveTime();
};

}// rocket
#endif // ROCKET_NET_TIMER_H