#include "timer_event.h"
#include "../common/log.h"
#include "../common/util.h"

// 类的参数设置上还存在顺序不大按照构造函数构造的顺序的问题，应该一一对应才是好codes！
namespace rocket {
TimerEvent::TimerEvent(
            int64_t interval, bool is_repeated , const std::function<void()>& task)
            : m_interval(interval), m_is_repeated(is_repeated), m_task(task), m_is_cancled(false)
            {
            // 下次任务的执行时间点 == 当前系统的时间 + 任务之间执行的时间间隔（时间统一是毫秒是Ms）
            m_arrive_time = getNowMs() + m_interval;
            if(m_is_repeated){
                m_timerEventType = REPEATABLE_TIMER_EVENT;
            }
            DEBUGLOG("success create a single-[%s]-TimerEvent, will execute at [%lld:%s]", TimerEventType2String(m_timerEventType).c_str()
            , m_arrive_time, origin13bitTimeStamp2RealTimeForMat(m_arrive_time).c_str());
}
void TimerEvent::setCancle(bool val){// true-取消一个定时任务,false-不取消
    m_is_cancled = val;  
}
bool TimerEvent::cancleRepeated(){ // 一个定时任务原来是重复执行的，但现在我们想给他取消重复执行这个熟属性就这么干！
    m_is_repeated = true;
    return true;
}
void TimerEvent::resetArriveTime(){// 重新set到达时间为当前最新的系统时间+初始化这个timerEvent时set好的时间间隔！
    m_arrive_time = getNowMs() + m_interval;
    // 这里原来写的是DEBUGLOG, 自己改成INFOLOG
    INFOLOG("success resetArriveTime for a single-[%s]-TimerEvent, will execute at [%lld:%s]", TimerEventType2String(m_timerEventType).c_str()
    , m_arrive_time, origin13bitTimeStamp2RealTimeForMat(m_arrive_time).c_str());
    // m_is_cancled = false;
}
const std::string TimerEvent::TimerEventType2String(TimerEventType timerEventType) const {
    std::string ret;
    switch (timerEventType){
    case NORMAL_TIMER_EVENT:
        ret = "Normal TimerEvent";break;
    case REPEATABLE_TIMER_EVENT:
        ret = "Repeatable TimerEvent";break;
        default: 
        ret = "Unknown TimerEvent Type";break;
    }
    return ret;
}
const std::string TimerEvent::getTimerEventType(){
    return TimerEventType2String(m_timerEventType);
} 
TimerEvent::~TimerEvent(){
    m_task = nullptr;
}
}// rocket