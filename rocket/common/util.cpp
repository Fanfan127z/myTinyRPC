#include "util.h"
#include <sys/syscall.h> // 获取线程号
#include <sys/types.h>
#include <sys/time.h>
#include <cstring>

namespace rocket {
static pid_t g_pid = 0;
static thread_local int g_thread_id = 0;

pid_t getPid(){ 
    if(g_pid != 0)return g_pid;
    return getpid();
}
pid_t getThreadId(){ 
    if(g_thread_id != 0)return g_thread_id;
    return syscall(SYS_gettid);
}

int64_t getNowMs() {// 获取当前时间(单位是 Ms毫秒)
    struct timeval tv;
    // 使用gettimeofday()获取当前时间，将秒数乘以1000并加上微秒数除以1000，
    // 从而得到毫秒数
    gettimeofday(&tv, nullptr);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


const std::string origin13bitTimeStamp2RealTimeForMat(int64_t timestamp, const char* time_format /* "%Y-%m-%d %H:%M:%S" */){
    // 因为timestamp是原始13位的时间戳，函数内部中间需要转换为10位的
    // time_t 10_bit_timestamp = 1627584000; // 1627584000 ==> 标识2021-07-30 00:00:00
    // time_t origin_13_bit_timestamp = 1686209168636; // 1686209168636 ==> 标识 2023-06-08 15:26:08
    time_t t_timestamp = timestamp / 1000; // 转换为 标识真实有效时间的 10位的 时间戳
    char str_time[30];
    memset(str_time, 0, sizeof(str_time));
    strftime(str_time, sizeof(str_time), time_format, localtime(&t_timestamp));
    // std::cout << "The time is: " << str_time << std::endl;
    return std::string(str_time);
}
    
  



}