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
    int ret = write(m_fd, buf, 8);
    if(ret != 8){
        ERRORLOG("failed to wakeup fd less than 8 bytes ,fd = [%d]\n", m_fd);
    }
    DEBUGLOG("success read 8 bytes\n");
}

}// rocket