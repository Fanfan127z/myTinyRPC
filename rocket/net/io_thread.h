#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include "eventloop.h"// use EventLoop class

namespace rocket {
/*
创建一个新IO线程 要do的工作是：
    1.创建一个新线程（pthread_create）
    2.在新线程里面 创建一个EventLoop，并完成初始化
    3.开启loop循环监听读写事件，并do（调用）对应的任务（回调函数）
    class {
        pthread_t m_thread;// 指向当前的线程
        pid_t m_thread_id;// 当前线程的id
        EventLoop m_event_loop;// 当前线程所持有的完成epoll事件整个流程的 Loop的对象
    }
*/

class IO_Thread{
private:
    // 注意了，Effective C++中指出，但凡是内置数据类型，都最好赋初值，否则会是一个undefine的值！
    pthread_t m_thread {-1};// 指向当前的线程(句柄)
    pid_t m_thread_id {-1};// 当前线程id
    EventLoop* m_event_loop {nullptr};// 当前线程所持有的 完成epoll事件整个流程的 Loop的对象(即当前IO线程的loop对象)

    sem_t m_init_semaphore;// 信号量，do同步用！
public:

    IO_Thread();
    ~IO_Thread();

    void TO_Thread_Join();// maybe used in future but now we do not use it
    
    // void* createEventLoop(void*);

private:
    // use private watch domain TODO:我个人认为会更加合理
    static void* Main(void* arg);
};
}// rocket


#endif // ROCKET_NET_IO_THREAD_H
