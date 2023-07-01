#include "util.h"
#include <sys/syscall.h> // 获取线程号
#include <sys/types.h>
#include <sys/time.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <arpa/inet.h>

namespace rocket {
static pid_t g_pid = 0;
static thread_local int g_thread_id = 0;

pid_t getPid(){ // 这个代码确实是参考人家写的才能写出来
    if(g_pid != 0)return g_pid;
    return getpid();
}
pid_t getThreadId(){ // 这个代码确实是参考人家写的才能写出来
    if(g_thread_id != 0)return g_thread_id;
    return syscall(SYS_gettid);
}

int64_t getNowMs() {// 获取当前时间的时间戳(单位是 Ms毫秒)
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
    
// 将int32_t类型的整数，从 网络字节序 转换为 主机字节序 
// 其实我没太懂这个，操作字符串类型的数据变成整型数据然后转为主机字节序？
int32_t convertInt32FromNetByte2HostByte(const char* buf){
    // int32_t ret = std::stoi(std::string(buf));
    int32_t ret;
    /*
        memcpy函数是C/C++中的一个内存拷贝函数，它的原型在<string.h>头文件中。
        它的功能是从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中。
        原型：void *memcpy(void *dest, const void *src, size_t n);
        参数：
        dest：目标内存地址
        src：源内存地址
        n：需要拷贝的字节个数
        返回值：返回指向dest的指针。
        原理：memcpy函数通过逐字节的复制来实现内存的拷贝。它不会因为遇到'\0'字符而停止复制，
        所以常常用来复制一些非文本的数据，如结构体、数组等。
    */
    memcpy(&ret, buf, sizeof(ret));
    return ntohl(ret);
} 


}