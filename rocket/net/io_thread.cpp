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

    // 唤醒等待的IO_Thread
    sem_post(&io_thread_ptr->m_init_semaphore);// 给当前线程的m_init_semaphore+1，以唤醒该线程继续往下执行!

    // 让IO 线程等待，直到我们主动地启动！

    DEBUGLOG("IO_Thread[%d] created, wait for start semaphore", io_thread_ptr->m_thread_id);

    sem_wait(&io_thread_ptr->m_start_semaphore);

    DEBUGLOG("IO_Thread[%d] now start loop", io_thread_ptr->m_thread_id);
    // 上面都是前置操作，下面的loop开启死循环需要 被唤醒
    // 唤醒这个等待中的新线程(need use 信号量semaphore)

    
    io_thread_ptr->m_event_loop->loop();// 开启loop循环，监听IO事件的触发！
    
    DEBUGLOG("IO_Thread[%d] end loop", io_thread_ptr->m_thread_id);
    return nullptr;
}

IO_Thread::IO_Thread(){
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);// 一定得成功创建了信号量，才能达到线程安全，才能是正确的代码，不然你根本没法走下去后面的codes的服务端流程！
    rt = sem_init(&m_start_semaphore, 0, 0);
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

EventLoop* IO_Thread::getEventLoop(){// 获取eventLoop对象！
    return m_event_loop;
}

void IO_Thread::start(){
    DEBUGLOG("Now invoke IO_Thread[%d] and start loop", m_thread_id);
    sem_post(&m_start_semaphore);// 给当前线程的m_start_semaphore +1，以唤醒该线程继续往下执行loop!
}
/*
    在C/C++的多线程编程环境下，pthread_join函数主要用于等待线程的结束。它是一个阻塞的函数，调用它的函数会被阻塞，直到被等待的线程结束为止。
    以下是需要使用pthread_join的情况：

    当我们需要在某个线程完成后才能继续执行其他任务时，需要使用pthread_join来等待该线程的完成。

    当我们需要获取线程的返回值时，也需要使用pthread_join。因为线程的返回值是通过pthread_join的第二个参数返回的。

    当我们需要确保线程已经结束，以防止产生僵尸线程时，也需要使用pthread_join。如果创建的线程计算结束后，没有被pthread_join等待，那么它就会成为一个僵尸线程，占用系统资源。

    使用pthread_join的好处：

    可以避免主线程在子线程结束前就退出，导致子线程成为孤儿线程，无法管理和回收其资源。

    可以获取线程的返回值，这对于一些需要处理线程计算结果的场景非常有用。

    可以避免僵尸线程的产生。如果不调用pthread_join或者pthread_detach，线程结束后，系统不会自动回收线程的资源，这就会导致僵尸线程的产生。

    可以同步线程的执行。在某些情况下，我们需要线程按照一定的顺序执行，这时就可以通过pthread_join来实现。
*/
void IO_Thread::join(){
    pthread_join(m_thread, nullptr);// 让 主线程调用IO_Thread的join函数 等待 当前的子线程的结束退出
}

IO_Thread::~IO_Thread(){
    /* 理论上来说，析构函数的代码不会被执行，因为上面的loop是死循环，
    我们的RPC-服务一旦开启，就一直在后台运行着！但养成这种打扫“战场”的好习惯 是 very 重要的编码思路！
    但如果你stop之后，可以跳出循环并退出当前的子线程（让封装好的IO线程安全合理的退出，very重要）*/

    m_event_loop->stop();   // 停止死循环

    sem_destroy(&m_init_semaphore);// 销毁信号量
    sem_destroy(&m_start_semaphore);
    // pthread_exit(nullptr);// 这个不需要
    join();// 等待IO_Thread线程返回，而不是让它在别的地方提前退出！
    // pthread_join(m_thread, NULL);
    if(m_event_loop != nullptr){
        delete m_event_loop;
        m_event_loop = nullptr;
    }
}

}// rocket