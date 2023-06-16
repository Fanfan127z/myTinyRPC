#include <unistd.h>
#include <error.h>
#include "wakeup_fd_event.h"
#include "../common/log.h"

namespace rocket {
WakeUpFdEvent::WakeUpFdEvent(int fd):FdEvent(fd){
    // init();
}
WakeUpFdEvent::~WakeUpFdEvent(){
    
}

// void WakeUpFdEvent::init(){// 实际上，我们只关心wakeup的可读事件！
//     m_read_callback =[this](){
//         char buf[8];
//         memset(buf, 0, sizeof(buf));
//         // int ret = 0;
//         // while( ret = read(m_fd, buf, 8) ){
//         //     if(ret == -1){// read()调用失败时 返回-1
//         //         // 读缓冲区没数据可读时（也即数据已经读完了的意思），errno == EAGAIN
//         //         if(errno == EAGAIN){// 数据读完
//         //             ERRORLOG("read this turn's data finished, over.\n");
//         //         }else{
//         //             ERRORLOG("read this turn's data error, error = [%d], error info:[%s]\n", strerror(errno));
//         //             exit(0);
//         //         }
//         //     }
//         // }
//         while(read(m_fd, buf, 8) != -1 && errno != EAGAIN ){
//             // 只要读buffer不出错切没有读完的话就继续读
//         }
//         DEBUGLOG("read full bytes from wakeup fd[%d]\n", m_fd);
//     };
// }
void WakeUpFdEvent::wakeup(){// 触发 读事件
    char buf[8] = {'a'};
    /* 
        简单write点东西到server程序端，
        以便于 服务器端,被epoll_wait识别出来fd的读缓冲区可读的时候，就触发返回，do你想do的任务回调函数callbcak
        其实这本身就是我服务端自己提供的代码，相当于是自己激活自己的意思了！只要你 开一个RPC-server调用，
        我就自己触发自己去检测触发client要do的任务（执行任务回调函数来达到这个目的）*/
    int ret = write(m_fd, buf, 8);
    if(ret != 8){
        ERRORLOG("failed to wakeup fd less than 8 bytes ,fd = [%d]", m_fd);
    }
    DEBUGLOG("success write 8 bytes from wakeup fd[%d]", m_fd);
}

}// rocket