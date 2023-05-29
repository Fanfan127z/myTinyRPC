#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H
#include <unistd.h>

// #include <pthread.h>

namespace rocket {

    pid_t getPid();// 获取进程号
    pid_t getThreadId();// 获取线程号

}

#endif // ROCKET_COMMON_UTIL_H