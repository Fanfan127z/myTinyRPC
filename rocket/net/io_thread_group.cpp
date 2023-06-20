#include "io_thread_group.h"
#include "../common/log.h"
namespace rocket{

IO_Thread_Group::IO_Thread_Group(size_t size):m_size(size){
    m_io_threads_group.resize(m_size);// 规定IO线程数组只有这么多个size的IO_Thread(IO线程)，以防止vector动态扩容，浪费时间空间复杂度
    for(int i = 0; i < m_size; ++i){
        m_io_threads_group[i] = std::make_shared<IO_Thread>();
    }
    INFOLOG("success create IO Thread Group, which size is [%d]", m_size);
}
IO_Thread_Group::~IO_Thread_Group(){
    // 恢复初始值
    m_size = 0;
    m_io_threads_group.clear();
    m_index = -1;
}
void IO_Thread_Group::start(){// 控制all的IO_Threads IO线程的loop循环的开始
    for(int i = 0; i < m_size; ++i){
        m_io_threads_group[i]->start();
    }
}
void IO_Thread_Group::join(){// 控制all的IO_Threads IO线程 被主线程等待回收
    for(int i = 0; i < m_size; ++i){
        m_io_threads_group[i]->join();
    }
}
std::shared_ptr<IO_Thread> IO_Thread_Group::getAvailableIO_Thread(){// 从当前的IO线程数组中获取一个IO线程！
    if(m_index == -1 || m_index == m_size){
        m_index = 0;
    }
    return m_io_threads_group[m_index++];// 获取完当前可用IO线程之后，可用线程下标m_index继续往后遍历
}
}// rocket