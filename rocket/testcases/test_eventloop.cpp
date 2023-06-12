#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <error.h>
#include <memory>

#include "../net/eventloop.h"
#include "../net/timer.h"
#include "../net/timer_event.h"
#include "../common/log.h"
#include "../common/util.h"
#include "../common/config.h"


int main(void){

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象

    rocket::EventLoop* eventloop = new rocket::EventLoop();
    // std::shared_ptr<rocket::EventLoop> eventloop = std::make_shared<rocket::EventLoop>();

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        ERRORLOG("listenfd == -1, socket error");
        exit(-1);
    }
    // 绑定 server端的 IP 和 Port
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;// IPV4
    serv_addr.sin_port = htons(10001);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 设置端口（可被）复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    int ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret == -1){
        perror("bind error");
        exit(-1);
    }
    // 监听
    ret = listen(listenfd, 128);
    if(ret == -1){
        perror("listen error");
        exit(-1);
    }
    // 现在只有监听的文件描述符
    // 所有文件描述符对应读写缓冲区状态都是委托内核去检测的epoll
    
    rocket::FdEvent event(listenfd);
    //  
    auto readable_task_cb = [listenfd](){
        struct sockaddr_in peer_client_addr;
        socklen_t client_addr_len = sizeof(peer_client_addr);
        /* TODO:  
            if 上面这行代码这里set为 0的话
            你就根本没法读取出对端客户端的地址值！因为这里面accept的源码是用这个len去读取对应len长的addr的！
            也即：accept的第3个参数是输入输出参数！除非你根本就 不关心 对端客户端的地址，你才可以传0的len进去！
            这个点非常重要！
       */ 
        int cfd = accept(listenfd, (struct sockaddr*)&peer_client_addr, &client_addr_len);
        const char * IP = inet_ntoa(peer_client_addr.sin_addr);// 大端网络字节序转换为小端主机字节序
        uint16_t  Port = ntohs(peer_client_addr.sin_port);
        DEBUGLOG("success get client connection [ip:port] = [%s:%d]", IP, Port);
    };
    event.listen(rocket::FdEvent::TriggerEvent::IN_EVENT, readable_task_cb);// 监听 读事件

    /* test timer
     TimerEvent(int64_t interval, bool is_repeated 
                // , const std::function<void()>& task);
    */
    int ii = 0;
    auto timer_task = [&ii](){
        INFOLOG("trigger a single-TimerEvent, count ii = [%d]", ii++);
    };
    rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(
        1000, true, timer_task
    );// 定义每秒钟执行一次的任务
    eventloop->addTimerEvent(timer_event);

    eventloop->addEpollEvent(&event);
    eventloop->loop();// 开启loop循环
    // delete eventloop;eventloop = nullptr;
    return 0;
}