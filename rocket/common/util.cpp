#include "util.h"
#include <sys/syscall.h> // 获取线程号
#include <sys/types.h>
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

}