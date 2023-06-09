#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H
// #include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <set>
#include <pthread.h>
#include <queue>
#include <functional>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>        // eventfd - create a file descriptor for event notification
#include <error.h>

#include "../common/log.h"      // 使用 日志 模块
#include "../common/util.h"     // 使用 常规函数 模块
#include "../common/mutex.h"    // 使用 互斥锁 模块，确保线程操作共享数据的安全
#include "../net/fdevent.h"    // 使用 fd 模块
#include "../net/wakeup_fd_event.h"    // 使用 wakeup_fd_event 模块
#include "../net/timer.h"       // 使用 Timer 模块
namespace rocket {
    
/* 
    使用Reactor模式的并使用epoll这种IO多路复用技术的类
*/
class EventLoop{
private:
    pid_t m_thread_id = {0};              // 保存线程id
    std::set<int> m_listen_fds;     // 监听的fd文件描述符
    int m_epoll_fd = {0};           // 操作当前的epoll实例的fd
    bool m_stop_flag = {false};     // loop暂停标志
    Mutex m_mutex;                  // 确保线程操作共享数据安全
    
    std::queue<std::function<void()>> m_pending_tasks;// 等待被执行的任务队列,任务队列里面存放的是任务函数，参数是空，返回值是void

    WakeUpFdEvent* m_wakeup_fd_event {nullptr};

    int m_wakeup_fd;                // 操作 唤醒任务do事情的fd！

    Timer* m_timer {nullptr};       // 定时器，定时执行 任务回调函数

public:
    EventLoop();

    void loop();                    // 循环调用epoll_wait（RPC服务的主函数程序）

    void wakeup();

    void stop();
    
    void addEpollEvent(FdEvent * event);
    void deleteEpollEvent(FdEvent * event);
    
    void addTimerEvent(TimerEvent::s_ptr event);   // 添加定时任务
    
    /*  bool isInLoopThread(); 
        （在添加epoll event之前）需要判断一下是否是当前的IO线程, if为别的线程添加的话就不能加，因为存在 条件竞争！
    */ 
    bool isInLoopThread() const;   
    void addTask(const std::function<void()>& cb, bool is_wake_up = false);   
    ~EventLoop();
private:
    void dealWakeUp();              // 处理wakeup的函数
    void initWakeUpFdEvent();
    void initTimer();       // 初始化定时器
};  

}
#endif // ROCKET_NET_EVENTLOOP_H