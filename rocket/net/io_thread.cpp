#include <functional>
#include <error.h>
#include <assert.h>// use assert()

#include "io_thread.h"
#include "../common/log.h"
#include "../common/util.h"
namespace rocket {

void* IO_Thread::Main(void* arg){
    // 此时，已经处于一个新的线程中了（if正确new 一个新thread）
    // 这3行代码是新线程(新的执行流)创建好前置EventLoop的前置工作的！
    IO_Thread* io_thread_ptr = static_cast<IO_Thread*>(arg);
    // 这些heap区资源肯定是一个进程中各个threads所共享的（就heap和数据区的资源属于共享资源，其他都是线程的私有资源，私有资源就无需关注）
    // 要么就用互斥锁来do，要么就用信号量来保护共享资源，以达到代码是 线程安全的 的目的！
    io_thread_ptr->m_event_loop = new EventLoop();
    io_thread_ptr->m_thread_id = getThreadId();

    sem_post(&io_thread_ptr->m_init_semaphore);// 给当前线程的m_init_semaphore+1，以唤醒该线程继续往下执行!
    // 上面都是前置操作，下面的loop开启死循环需要 被唤醒
    // 唤醒这个等待中的新线程(need use 信号量semaphore)
    io_thread_ptr->m_event_loop->loop();// 开启loop循环，监听IO事件的触发！

}

IO_Thread::IO_Thread(){
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);// 一定得成功创建了信号量，才能达到线程安全，才能是正确的代码，不然你根本没法走下去后面的codes的服务端流程！

    int ret = pthread_create(&m_thread, nullptr, &IO_Thread::Main, this);
    if(ret != 0){
        ERRORLOG("failed to create a thread, error = [%d], error info:[%s]", errno, strerror(errno));
        exit(0);// 异常，提前退出程序！
    }
    // wait, 直到新线程执行完 Main 函数的前置，即初始化EventLoop的前置操作（loop是死循环了）
    sem_wait(&m_init_semaphore);// 一定等待信号量的值+1了，被唤醒了才往下继续执行codes！
    DEBUGLOG("New IO_Thread(thread id : [%d]) create success", m_thread_id);
}
/*
    一旦涉及到后续加入多线程/线程池的代码，就必须要使用pthread_join() 
    来达到满足我们效果的多线程服务代码！（这个点如果有疑惑，可以去本地/home/ubuntu/test/test_sem.cpp文件中去测试，代码基本都写好了）

    一旦忘记了pthread_join 或者 使用了pthread_detach,就很容易会造成线程不安全的代码！
*/
// maybe used in future but now we do not use it
void IO_Thread::TO_Thread_Join(){
    pthread_join(m_thread, nullptr);// 让主线程等待子线程返回
}
IO_Thread::~IO_Thread(){
    /* 理论上来说，析构函数的代码不会被执行，因为上面的loop是死循环，
     但如果你stop之后，可以跳出循环并退出当前的子线程（让封装好的IO线程安全合理的退出，very重要）*/
    if(m_event_loop != nullptr){
        delete m_event_loop;
        m_event_loop = nullptr;
    }
    sem_destroy(&m_init_semaphore);// 销毁信号量
    
    // pthread_exit(nullptr);// 这个不需要
}

}// rocket