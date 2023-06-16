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

#include "../common/log.h"
#include "../common/util.h"
#include "../common/config.h"

#include "../net/eventloop.h"
#include "../net/timer.h"
#include "../net/timer_event.h"
#include "../net/io_thread.h"
#include "../net/io_thread_group.h"

// 虽然我自己还没有完全理解这个eventloop的整个流程的all代码，但是也89不离10，我只需要再多仔细看我的项目代码
// 然后多看看epoll_wait相关的原理，就差不多能完全吃透这个eventloop的整个模块的代码了！
void test_io_thread(){

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
    
    rocket::FdEvent event(listenfd);// 监听 客户端连接 的事件！
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


    // 我要在我的EventLoop.h/.cpp中，把监听的事件类型要说清楚啊，到底是一开始初始化时的listenfd监听的事件呢还是别的真正do
    // RPC通信的fd触发的事件！
    // 这个还是得看我之前写的/test/test_epoll_xxx.cpp文件中的代码！


    /* test timer
     TimerEvent(int64_t interval, bool is_repeated 
                // , const std::function<void()>& task);
    */
    int ii = 0;
    // 创建一个定时运行的任务！
    auto timer_task = [&ii](){
        INFOLOG("{ trigger a single-TimerEvent, count ii = [%d] }", ii++);
    };
    rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(
        1000, true, timer_task
    );// 定义每秒钟执行一次的任务(任务内容就是打印"trigger a single-TimerEvent, count ii = [%d]"的信息)
    // 具体直接看logs中的
    // 一个trigger a single-TimerEvent, count ii = [x] ==> 下一个trigger a single-TimerEvent, count ii = [x+1]
    // 中间肯定是间隔1s的！我打印的日志时间就能够说明问题！很容易看的！

    // rocket::IO_Thread io_thread;// 创建IO_Thread对象（保存在该函数的栈区，当该函数运行周期结束之后,io_thread的析构函数也会join等待该函数返回回收资源）

    // io_thread.getEventLoop()->addTimerEvent(timer_event);

    // io_thread.getEventLoop()->addEpollEvent(&event);// 添加监听客户端连接 事件
    // io_thread.start();// 控制：开启loop循环
    // io_thread.join();// 让 主线程调用IO_Thread的join函数 等待 当前的子线程的结束退出
    // // 等待IO_Thread线程返回，而不是让它在别的地方提前退出！

    /*
        test codes for io_thread_group:
        第一个IO线程既会监听并触发客户端连接的事件，也会触发定时任务事件
        第二个IO线程只会触发定时任务事件
    */
    rocket::IO_Thread_Group io_thread_group(2);// 创建含有2个IO线程的IO线程组
    std::shared_ptr<rocket::IO_Thread> io_thread1 = io_thread_group.getAvailableIO_Thread();
    io_thread1->getEventLoop()->addEpollEvent(&event);// 添加 客户端的监听事件
    io_thread1->getEventLoop()->addTimerEvent(timer_event);// 添加 定时任务事件
    std::shared_ptr<rocket::IO_Thread> io_thread2 = io_thread_group.getAvailableIO_Thread();
    io_thread2->getEventLoop()->addTimerEvent(timer_event);// 添加 定时任务事件
    io_thread_group.start();
    io_thread_group.join();
}
// 说句实话，我自己现在写的代码的各种INFOLOG或者DEBUGLOG的logs非常混乱，其实很难查看！

// 我后面需要统一一下这个log的形式，以便于查看程序是否在正常运行当中！
int main(void){

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象
    

    test_io_thread();

    // rocket::EventLoop* eventloop = new rocket::EventLoop();
    // std::shared_ptr<rocket::EventLoop> eventloop = std::make_shared<rocket::EventLoop>();

    // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if(listenfd == -1){
    //     ERRORLOG("listenfd == -1, socket error");
    //     exit(-1);
    // }
    // // 绑定 server端的 IP 和 Port
    // struct sockaddr_in serv_addr;
    // memset(&serv_addr, 0, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;// IPV4
    // serv_addr.sin_port = htons(10001);
    // serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // // 设置端口（可被）复用
    // int opt = 1;
    // setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // // 绑定端口
    // int ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // if(ret == -1){
    //     perror("bind error");
    //     exit(-1);
    // }
    // // 监听
    // ret = listen(listenfd, 128);
    // if(ret == -1){
    //     perror("listen error");
    //     exit(-1);
    // }
    // // 现在只有监听的文件描述符
    // // 所有文件描述符对应读写缓冲区状态都是委托内核去检测的epoll
    
    // rocket::FdEvent event(listenfd);
    // //  
    // auto readable_task_cb = [listenfd](){
    //     struct sockaddr_in peer_client_addr;
    //     socklen_t client_addr_len = sizeof(peer_client_addr);
    //     /* TODO:  
    //         if 上面这行代码这里set为 0的话
    //         你就根本没法读取出对端客户端的地址值！因为这里面accept的源码是用这个len去读取对应len长的addr的！
    //         也即：accept的第3个参数是输入输出参数！除非你根本就 不关心 对端客户端的地址，你才可以传0的len进去！
    //         这个点非常重要！
    //    */ 
    //     int cfd = accept(listenfd, (struct sockaddr*)&peer_client_addr, &client_addr_len);
    //     const char * IP = inet_ntoa(peer_client_addr.sin_addr);// 大端网络字节序转换为小端主机字节序
    //     uint16_t  Port = ntohs(peer_client_addr.sin_port);
    //     DEBUGLOG("success get client connection [ip:port] = [%s:%d]", IP, Port);
    // };
    // event.listen(rocket::FdEvent::TriggerEvent::IN_EVENT, readable_task_cb);// 监听 读事件

    // /* test timer
    //  TimerEvent(int64_t interval, bool is_repeated 
    //             // , const std::function<void()>& task);
    // */
    // int ii = 0;
    // auto timer_task = [&ii](){
    //     INFOLOG("trigger a single-TimerEvent, count ii = [%d]", ii++);
    // };
    // rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(
    //     1000, true, timer_task
    // );// 定义每秒钟执行一次的任务
    // // 具体直接看logs中的
    // // 一个trigger a single-TimerEvent, count ii = [x] ==> 下一个trigger a single-TimerEvent, count ii = [x+1]
    // // 中间肯定是间隔1s的！我打印的日志时间就能够说明问题！很容易看的！
    // eventloop->addTimerEvent(timer_event);

    // eventloop->addEpollEvent(&event);
    // eventloop->loop();// 开启loop循环
    // // delete eventloop;eventloop = nullptr;
    return 0;
}