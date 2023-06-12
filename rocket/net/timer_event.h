#ifndef ROCKET_NET_TIMER_EVENT_H
#define ROCKET_NET_TIMER_EVENT_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <memory>


namespace rocket{
/*
    2.4.1 TimeEvent 定时任务 事件
        1. 指定任务的到达时间点 arrive_time, 单位是ms毫秒
        2. 到达下一次任务执行的时间间隔 interval, 单位是ms毫秒
        3. 判断该任务是否是需要周期性执行的任务 is_repeated，比如一个任务我们need反复执行10次，时间间隔是5ms，就要set为true
        4. 判断是否取消该定时器的标志 is_cancled
        5. task

        方法:
        cancle(): 取消一个定时任务
        cancleRepeated(): 一个定时任务原来是重复执行的，但现在我们想给他取消重复执行这个属性就这么干！

*/
class TimerEvent{
private:
    
    int64_t m_interval {0};         // ms,到达下一次任务执行的时间间隔 interval
    bool m_is_repeated {false}; // 判断该任务是否是需要周期性执行的任务 is_repeated，比如一个任务我们反复执行10次，时间间隔是5ms
    std::function<void()> m_task {nullptr}; // 任务(函数指针)
    bool m_is_cancled {false};  // 判断是否取消该定时器的标志 is_cancled
    int64_t m_arrive_time;      // ms, 指定任务的到达时间点 arrive_time

    enum TimerEventType{
        NORMAL_TIMER_EVENT, // 正常timerEvent定时任务事件，不重复执行（默认, 缺省）
        REPEATABLE_TIMER_EVENT,// 需要被重复执行的timerEvent定时任务事件
    };
    TimerEventType m_timerEventType = NORMAL_TIMER_EVENT; // 定时任务事件的属性(默认是正常的不被重复执行的！)
    
public:
    typedef std::shared_ptr<TimerEvent> s_ptr;
    TimerEvent(int64_t interval, bool is_repeated 
                , const std::function<void()>& task);
    ~TimerEvent();
    const std::string TimerEventType2String(TimerEventType timerEventType)const;

    const std::string getTimerEventType();
    inline int64_t getArriveTime() const{ return m_arrive_time; }
    void resetArriveTime();// 重新set到达时间
    inline bool isCancled() const { return m_is_cancled; }
    inline bool isRepeated() const { return m_is_repeated; }
    inline const std::function<void()> getCallBackTask() const{ return m_task; }
    
    
    void setCancle(bool val);          // 取消一个定时任务
    bool cancleRepeated();  // 一个定时任务原来是重复执行的，但现在我们想给他取消重复执行这个熟属性就这么干！


};


}//

#endif // ROCKET_NET_TIMER_EVENT_H