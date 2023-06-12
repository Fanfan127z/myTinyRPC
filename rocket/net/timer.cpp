#include <sys/timerfd.h> // linux下的自带的定时器, use timerfd_create and timerfd_settime
#include <functional> // use std::bind()
#include <vector>
#include <errno.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <string.h> // use memset

#include "timer.h"
#include "../common/log.h"// use log
#include "../common/util.h" 

namespace rocket{

Timer::Timer() : FdEvent(){
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(m_fd == -1){
        ERRORLOG("timerfd_create error, error = [%d], error info = [%s]", errno, strerror(errno));
    }
    DEBUGLOG("Timer fd = [%d]", m_fd);

    /* 
        std::bind(Function, arg1, arg2, ...);
        其中Function是要绑定的函数对象，arg1、arg2等是要绑定的参数，有几个arg取决于function的参数表。返回值是一个新的可调用对象，
    */ 
    // 这里把 Timer所绑定的fd 能够触发的可读事件 放到EventLoop的epoll_wait里面去 被监听了！ 
    // 或者说，把fd 的可读事件放到EventLoop中去 被监听了！

    listen(rocket::FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
    // 传入this指针是因为在类的成员函数中，你不传这个this的话就调用不到属于Timer类的这个onTimer()函数
    // 编译器只会给你去调用本源文件下的全局 或局部 onTimer()函数，但是显而易见，这是没有的事情，so会 lead to error！
}


// re-set Timer类 所管理的第一个timerEvent的到达时间
void Timer::resetArriveTime(){
    ScopeMutex<Mutex> lock(m_mutex);
    
    if(m_pending_events.empty()){
        return;// if 没有等待处理的event就直接退出！
    }
    int64_t now = getNowMs();// 获取当前的系统时间（毫秒Ms）
    auto it = m_pending_events.begin();
    int64_t interval = 0;
    // TODO: 我其实不太理解这里这个代码逻辑！
    if(( it->second->getArriveTime() > now )){
        interval = it->second->getArriveTime() - now;// 到达时间-当前时间 才是真正的时间间隔，让当前timerEvent不要等那么久了
    } else {
        // 这个任务超时了都没有给执行，就等100ms 就 马上触发可读事件 进而去执行下一个任务！
        interval = 100;// 0.1s ==  0.1*1000ms == 100ms
    }
    struct timespec ts;
    memset(&ts, 0, sizeof(ts)); // 初始化，防止出现undefined问题！
    ts.tv_sec = interval / 1000;// 秒s
    ts.tv_nsec = (interval % 1000) * 1000000;// 纳秒ns：(interval % 1000)得到微妙，然后 *10^6得到纳秒
    
    struct itimerspec value;
    memset(&value, 0, sizeof(value)); // 初始化，防止出现undefined问题！
    value.it_value = ts;
    int ret = timerfd_settime(m_fd, 0, &value, nullptr);
    if(ret != 0){
        ERRORLOG("timerfd_settime error, error = [%d], error info = [%s]", errno, strerror(errno));
    }
    DEBUGLOG("success reset Timer class's first timerEvent's arrivetime to [%lld:%s]", now + interval
    , origin13bitTimeStamp2RealTimeForMat(now + interval).c_str());
    // 打印一下新set的到达执行任务的时间 = now + interval
    // 此时如果set时间成功的话，那么我们的m_fd就会在这个指定的时间内去触发 读事件！
    // 触发之后EventLoop中的epoll_wait就能够监听到了！
}
// 我没有看懂这个函数的 重新set超时时间 的意思！
// 如果 当前timerEvent事件的到达时间如果 >于 timerEvent事件队列中即将最先被执行的第一个事件任务的到达时间
// 就重新设置超时时间 并 把新的timerEvent加入到timerEvent事件队列中！
bool Timer::addTimerEvent(const rocket::TimerEvent::s_ptr& event){
    bool is_reset_timerfd = false;// 是否需要重新设置超时时间 的 标志

    ScopeMutex<Mutex> lock(m_mutex);
    if( m_pending_events.empty() ){
        is_reset_timerfd = true;
    } else {
        auto it = m_pending_events.begin();
        if(( it->second->getArriveTime() > event->getArriveTime() )){
            is_reset_timerfd = true;// 说明此时新增加的timerEvent定时器事件是all事件中最快即将执行的！
        }
    }
    m_pending_events.insert({event->getArriveTime(), event});
    lock.unlock();
    // print debug logs
    DEBUGLOG("success [ADD] a single-[%s]-TimerEvent at arrivetime = [%lld:%s]", event->getTimerEventType().c_str()
    , event->getArriveTime(), origin13bitTimeStamp2RealTimeForMat(event->getArriveTime()).c_str());

    if(is_reset_timerfd){
        resetArriveTime();// re-set Timer类 所管理的第一个timerEvent的到达时间
    }
    return true;
}
bool Timer::deleteTimerEvent(const rocket::TimerEvent::s_ptr& event){
    event->setCancle(true);// 设置 定时器的 取消标志为true，代表后续不再触发这个 定时器了！
    ScopeMutex<Mutex> lock(m_mutex);
    // m_pending_events.erase(event->getArriveTime());
    // multimap.erase(_key)会按照_key值把multimap中的all的key值==_key的pair对组都delete掉
    // 无论有多少个重复的key值，只要==_key都会给delete掉！

    auto begin = m_pending_events.lower_bound(event->getArriveTime());// multimap中第一个arrivetime == event->arrivetime的 pair对
    auto end = m_pending_events.upper_bound(event->getArriveTime());// multimap中最后一个arrivetime == event->arrivetime的 pair对
    auto it = begin;
    for(;it != end;++it){
        if(it->second == event){// if遇到已经存在的timerEvent事件，就停止寻找
            break;
        }
    }
    if(it != end){// if存在该事件，就删除之。
        m_pending_events.erase(it);
    }
    lock.unlock();// 其实这句code 可有可无
    DEBUGLOG("success [DELETE] a single-[%s]-TimerEvent at arrivetime = [%lld:%s]", event->getTimerEventType().c_str()
    , event->getArriveTime(), origin13bitTimeStamp2RealTimeForMat(event->getArriveTime()).c_str());
    return true;
}

/*
    当Timer所绑定的fd被epoll_wait检测到有IO事件触发(发生)了之后，就需要执行任务函数
    那么这个任务函数(或者说方法就是onTimer)
*/
/*
    onTimer是该类 中 最关键的函数！
    作用是：在我们触发的可读事件上需要调用的回调函数
*/

void Timer::onTimer(){ 
    // TODO: 准备明天来写！
    // 处理缓冲区数据，防止下一次 继续触发 可读事件
    DEBUGLOG("ontimer");
    char buf[8];
    memset(buf, 0, 8);
    while(true){
        if(read(m_fd, buf, 8) == -1 && errno == EAGAIN ){
            break;// 读完 且 出错了，才跳出循环
        }
        // 只要读buffer不出错 且 没有读完的话就继续读
    }

    // 执行定时任务
    int64_t now = getNowMs();// 先获取当前系统时间
    /* 
        std::vector<TimerEvent::s_ptr> tmps;
        保存 指向 到期的任务(TimerEvent) 的shared_ptr指针，方便后续判断是否需要重复insert进去任务队列
        （因为有一些任务是定期执行的任务，即使当前时间执行了一次，后面还要再加回来再过段时间再pop处理执行再push回去循环反复的！）
    */
    std::vector<TimerEvent::s_ptr> tmps;
    /*
        std::vector< std::pair<int64_t, std::function<void()>> > tmp_tasks_queue;
        保存到期的任务的arrivetime和到期之后需要执行的回调函数（也即：任务，任务就是回调函数）
    */
    std::vector< std::pair<int64_t, std::function<void()>> > tmp_tasks_queue;

    ScopeMutex<Mutex> lock(m_mutex);
    auto begin = m_pending_events.begin();
    auto it = begin;
    for(; it != m_pending_events.end(); ++it){
        if( it->first <= now){
            // 当前任务的待执行时间 小于等于当前时间，就说明该任务到期了，也即该任务应该需要我们去执行了 的意思！
            // 当然，还需要同步判断上该任务是否给取消，如果已经给取消那肯定就不需要执行了！
            if(!it->second->isCancled()){
                tmps.push_back(it->second);
                // emplace_back()别乱用，否则你move掉原来的东西之后，你后面再删除就是释放已经给到tmps中对应元素权限的内存
                // 这样再使用tmps你就会出错了！so 涉及到std::move的方法emplace_back必须要小心使用！
                tmp_tasks_queue.push_back(
                    std::make_pair(it->second->getArriveTime(),it->second->getCallBackTask()));
            }
        } else {
            /* 此时，说明事件队列m_pending_events中的当前it和后面的[it,end]的任务们都是在未来 等时间到了才会被执行
            现在还不是执行这些事件对应的任务的时候(因为我们都是按照arrivetime进行升序排序的，越靠近begin前面的it就越有可能时间到了被执行，
            越到后面的it就越不可能到了arrivetime被执行,so当前和后面的events就更不可能被执行了，因为arrivetime时间还没到呢！) */
            break;
        }
    }
    // 先删除掉该被执行的任务！TODO:(我写完测试代码之后，运行时 出现的 segment default的报错位置在这里！)
    // m_pending_events.erase(begin, it);// 原本的代码是begin,这行代码是会报Segmentation fault (core dumped)错的！
    m_pending_events.erase(m_pending_events.begin(), it);// erase [first, last)左闭右开区间内的all elements from container
    lock.unlock();
    // 再需要把重复的Event，再次添加到事件任务队列m_pending_events中
    std::vector<rocket::TimerEvent::s_ptr>::iterator ite = tmps.begin();
    for( ;ite != tmps.end(); ++ite){
        if( (*ite)->isRepeated() ){
            int64_t arriveTime = (*ite)->getArriveTime();// 到达时间的时间戳
            // INFOLOG("success resetArriveTime for repeatable TimerEvent, will execute at [%lld:%s]", arriveTime
            //     , origin13bitTimeStamp2RealTimeForMat(arriveTime).c_str());
            // 跳转arrivetime之后再次添加到任务队列中！
            (*ite)->resetArriveTime();// reset单独每个TimerEvent事件的到达执行时间
            addTimerEvent(*ite);
        }
    }
    // 保险起见再set一下arrivetime（我没有看懂）
    resetArriveTime();// reset Timer这个管理all的TimerEvent事件的到达执行时间
    // 此时，真正执行 可读事件触发后 需要执行的回调函数
    for(auto task : tmp_tasks_queue){
        if(task.second != nullptr){
            task.second();// 回调函数非空，就execute
            INFOLOG("success execute trigger TimerEvent function");
        }
    }
}
Timer::~Timer(){
    

}

}// rocket