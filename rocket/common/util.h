#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H
#include <unistd.h>
#include <string>
#include <ctime>

// #include <iostream>
#include <sys/types.h> // 用 getNowMs
// #include <pthread.h>

namespace rocket {

    
    pid_t getPid();// 获取进程号
    pid_t getThreadId();// 获取线程号
    int64_t getNowMs();// 获取当前时间(单位是 Ms毫秒)

    const std::string origin13bitTimeStamp2RealTimeForMat(// 转换时间戳为真实时间（人类能够看懂的）
        int64_t timestamp, const char* time_format="%Y-%m-%d %H:%M:%S");


}

#endif // ROCKET_COMMON_UTIL_H