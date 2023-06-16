#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include "io_thread.h"
#include <vector>
#include <memory>

/*
    IO_Thread组是对IO_Thread的进一步的封装
    本质上，是TO_Thread的数组

    注意：由于这个IO线程组的类对象的成员和方法，只会在我们RPC服务的这个主程序main中去调用，因此没有别的线程来调用它。
   也即是，没有 需要维护线程安全 的场景，因此，无需写任何互斥操作，
*/
namespace rocket{

class IO_Thread_Group{
private:
    size_t m_size {0};// 数组大小
    std::vector<std::shared_ptr<IO_Thread>> m_io_threads_group;// IO线程组
    int m_index {-1};// IO线程组中 可用IO线程 的下标
public:
    IO_Thread_Group(size_t size);
    ~IO_Thread_Group();
    void start();// 控制all的IO_Threads IO线程的loop循环的开始结束
    void join();// 控制all的IO_Threads IO线程 被主线程等待回收
    std::shared_ptr<IO_Thread> getAvailableIO_Thread();// 从当前的IO线程数组中获取一个可用的IO线程！

};

}// rocket
#endif // ROCKET_NET_IO_THREAD_GROUP_H