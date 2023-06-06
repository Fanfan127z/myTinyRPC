
#include "eventloop.h"

#define ADD_TO_EPOLL() \ 
    auto it = m_listen_fds.find(event->getFd());\
    int op = EPOLL_CTL_ADD;\
    if(it != m_listen_fds.end()){\
        op = EPOLL_CTL_MOD;\
    }\
    struct epoll_event tmp_ev = event->getEpollEvent();\
    INFOLOG("epoll_event.events = [%d]", (int)tmp_ev.events); \ 
    int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp_ev);\
    if(ret == -1){\
        ERRORLOG("failed to epoll_ctl when add fd [%d], error = [%d], error info:[%s]", event->getFd(), errno, strerror(errno));\
    }\
    else DEBUGLOG("add epoll event success, fd = [%d]", event->getFd());\

// 视频的大佬教的时候写为DELETE_TO_EPOLL()了，我觉得他老是忘记这些细枝末节，但是问题不大！

#define DELETE_FROM_EPOLL() \
     auto it = m_listen_fds.find(event->getFd());\
    if(it == m_listen_fds.end()){\
        return; \
    }\
    int op = EPOLL_CTL_DEL; \
    struct epoll_event tmp_ev = event->getEpollEvent();\
    int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp_ev);\
    if(ret == -1){\
        ERRORLOG("failed to epoll_ctl when delete fd [%d], error = [%d], error info:[%s]", event->getFd(), errno, strerror(errno));\
    }\
    else DEBUGLOG("delete epoll event success, fd = [%d]", event->getFd());\


// 最后项目差不多做好的时候再把各种日志的打印都规范化一下，学一下SRS那个log！
namespace rocket {

// 用全局的线程变量，判断某个线程是否已经创建了eventloop!
static thread_local EventLoop* t_current_eventloop = nullptr;// 显然，肯定是用shared_ptr来管理好过裸指针来do！

// epoll最大的 超时时间
static int g_epoll_max_timeout = 10000;// 这里定为10s,超过这个时间就不再阻塞epoll_wait

// epoll最大的 监听事件数量
static int g_epoll_max_events = 10;

EventLoop::EventLoop(){
    if(t_current_eventloop != nullptr){
        ERRORLOG("failed to create event loop, this thread has created event loop");// 打印error日志
        exit(0);// 异常，提前退出程序！
    }
    m_thread_id = getThreadId();
    
    m_epoll_fd = epoll_create(10);// 创建epoll实例
    if(m_epoll_fd == -1){
        ERRORLOG("failed to create epoll, epoll_create called error, error = [%d], error info:[%s]", strerror(errno));// 打印error日志
        exit(0);
    }

    initWakeUpFdEvent();

    INFOLOG("successfully created event loop in thread [%d]", this->m_thread_id);// 打印info程序运行信息的日志
    t_current_eventloop = this;
}
void EventLoop::initWakeUpFdEvent(){
    /*  
        int eventfd(unsigned int __count, int __flags):
        initval参数是eventfd对象的初始值;
        flags参数用于设置eventfd对象的行为;
        在这个例子中，我们将initval设置为0，flags设置为EFD_NONBLOCK，表示创建一个 非阻塞的 初始值为0的 eventfd。
    */ 
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK); 
    if(m_wakeup_fd == -1){
        ERRORLOG("failed to create m_wakeup_fd, eventfd create error, error = [%d], error info:[%s]", strerror(errno));// 打印error日志
        exit(0);
    }

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    // 实际上，我们只关心wakeup的可读事件！
    auto m_read_callback = [this](){
        char buf[8];
        memset(buf, 0, sizeof(buf));// 初始化
        while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN ){
            // 只要读buffer不出错切没有读完的话就继续读
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    };
    m_wakeup_fd_event->listen(FdEvent::TriggerEvent::IN_EVENT, m_read_callback);
    addEpollEvent(m_wakeup_fd_event);// 这里面已经do了下面注释部分的工作了！
    // struct epoll_event event;
    //  // 检测 m_wakeup_fd 的读缓冲区 是否有数据
    // event.events = EPOLLIN;// 即：只需要监听m_wakeup_fd这个fd的读事件就行！
    // event.data.fd = m_wakeup_fd;
    // int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wakeup_fd, &event);
    // if(ret == -1){
    //     ERRORLOG("failed to epoll control, epoll_ctl eventfd called error, error = [%d], error info:[%s]", strerror(errno));// 打印error日志
    //     exit(0);
    // }
}
void EventLoop::loop(){// 循环调用epoll_wait（RPC服务的主函数程序）
    while(!m_stop_flag){
        // 先拿出任务队列 中的 任务们
        ScopeMutex<Mutex> lock(m_mutex);
        // TODO: 其实下面这2行代码可以直接省略，然后后面while(!m_pending_task.empty())... lock.unlock();
        //       这样子写也是ok的！但大佬这么写应该有大佬自己的原因！反转效果的话这2种写法都是一样的！但上面那种更加好理解！
        std::queue<std::function<void ()>> tmp_tasks;
        m_pending_task.swap(tmp_tasks);
        // TODO: 这就相当于是用tmp_tasks代替m_pending_task来do任务函数出队列的操作
        //       这样子do可以更加让我们写粒度更加低, 一定程度上增加了程序运行的效率！
        // 同时，此时m_pending_task就会给clear掉！相当于此时tmp_tasks是原来的m_pending_task
        // m_pending_task == 空！
        // 这里为啥要这样子呢，没搞懂，这样子tmp_task和m_pending_task都是一样的啊，为什么要swap呢？
        
        lock.unlock();
         /*TODO:  疑问：可是如果我加上互斥锁这个问题不就不会出现了吗？为什么还要swap呢？
            是的，如果你在执行任务的过程中使用互斥锁保护m_pending_task队列，
            确实可以避免新任务在执行过程中被加入到队列中。这样，你可以确保在执行任务时，队列中的任务不会发生变化，从而避免了意外的行为。
            然而，使用swap的方法仍然具有一定的优势。首先，它可以减少锁的持有时间，因为在执行任务时不需要持有锁。这样可以提高程序的并发性能。其次，使用swap方法可以确保在执行任务过程中，新加入的任务不会立即被执行，
            而是等待下一轮执行。这有助于控制任务的执行顺序和节奏。
            其实，我还是不太满意这个答案！难道这种操作规范的写法，是在modern C++中给出的answer和指导指引么？
            （我的这个疑惑待后续deal！）
        */
       // 执行任务们
        while(!tmp_tasks.empty()){
            std::function<void()> task = tmp_tasks.front();
            tmp_tasks.pop();
            // 下面这一步不执行的话，很容易出事的！！！
            if(task){ // std::function<void()>所封装的 任务函数 入口调用的地址非空，我才执行！
                task();// 执行 该执行的任务！
            }
        }
        // lock.unlock();
        // 1.取得下次定时任务的时间，与设定time_out取最大值，即若下次定时任务时间超过1s就取下次定时任务的时间为超时时间，否则取1s
        // int time_out = max(g_epoll_max_timeout, getNextTimeCallBack());
        int time_out = g_epoll_max_timeout; 
        // 2.调用epoll等待事件的发生，超时时间为上述的time_out
        struct epoll_event result_events[g_epoll_max_events];
        // int size = sizeof(result_events) / sizeof(struct epoll_event);

        DEBUGLOG("now begin to epoll wait");
        int num = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, time_out);
        DEBUGLOG("now end epoll wait");

        if(num < 0){// num : epoll_wait返回的是就绪事件个数
            ERRORLOG("failed to loop, epoll_wait error, error = [%d], error info:[%s]", errno, strerror(errno));// 打印error日志
            exit(0);// 异常，提前退出程序！
        }else {// epoll_wait调用成功
            if(num > 0){
                for(int i=0;i < num;++i){
                    struct epoll_event trigger_event = result_events[i];// 取出触发的事件
                    FdEvent* fd_event = static_cast<rocket::FdEvent *>(trigger_event.data.ptr);
                    if(fd_event == nullptr)continue;// 没法处理
                    if(trigger_event.events & EPOLLIN){// 如果是 触发 可读事件的话
                        DEBUGLOG("fd[%d] trigger EPOLLIN event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::TriggerEvent::IN_EVENT));// 就把回调函数添加到任务队列中
                    }
                    if(trigger_event.events & EPOLLOUT){// 如果是 触发 可写事件的话
                        DEBUGLOG("fd[%d] trigger EPOLLOUT event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::TriggerEvent::OUT_EVENT));// 就把回调函数添加到任务队列中
                    }
                }// for
            }// num > 0
        }// num >= 0 
    }// while
}

void EventLoop::wakeup(){
    m_wakeup_fd_event->wakeup();
}

void EventLoop::stop(){
    m_stop_flag = true;
}

// （在添加epoll event之前）需要判断一下是否是当前的IO线程
// if为别的线程添加的话就不能加，因为存在 条件竞争！
bool EventLoop::isInLoopThread() const{  
    return m_thread_id == getThreadId();
}   
void EventLoop::addEpollEvent(FdEvent * event){
    if(isInLoopThread()){
        ADD_TO_EPOLL();// 直接封装为一个 宏 了！
        // 判断出是当前线程就是直接添加epoll event就ok
        // auto it = m_listen_fds.find(event->getFd());
        // int op = EPOLL_CTL_ADD;// 默认操作是 添加
        // if(it != m_listen_fds.end()){
        //     // 已经添加过了
        //     op = EPOLL_CTL_MOD;// 改默认操作为 修改
        // }
        // struct epoll_event tmp_ev = event->getEpollEvent();
        // int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp_ev);
        // if(ret == -1){
        //     ERRORLOG("failed to epoll_ctl when add fd [%d], error = [%d], error info:[%s]", event->getFd(), errno, strerror(errno));
        // }
    } else {
        // if不为当前线程的话，就需要 用到 回调函数
        auto callback = [this, event](){
            ADD_TO_EPOLL();
        };
        // auto callback = [this, event](){
        //     auto it = m_listen_fds.find(event->getFd());
        //     int op = EPOLL_CTL_ADD;// 默认操作是 添加
        //     if(it != m_listen_fds.end()){
        //         // 已经添加过了
        //         op = EPOLL_CTL_MOD;// 改默认操作为 修改
        //     }
        //     struct epoll_event tmp_ev = event->getEpollEvent();
        //     int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp_ev);
        //     if(ret == -1){
        //         ERRORLOG("failed to epoll_ctl when add fd [%d], error = [%d], error info:[%s]", event->getFd(), errno, strerror(errno));
        //     }
        // };
        addTask(callback, true);// 添加事件的时候，我们是希望尽快把 任务 添加进去 任务队列里面的！
    }
    
}
void EventLoop::addTask(const std::function<void()>& cb, bool is_wake_up /* = false*/){
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_task.push(cb);
    lock.unlock();
    if(is_wake_up){
        wakeup();
    }
}
void EventLoop::deleteEpollEvent(FdEvent * event){
    if(isInLoopThread()){
        DELETE_FROM_EPOLL()
        // 直接封装为一个 宏 了！
        // 判断出是当前线程就是直接添加epoll event就ok
        // auto it = m_listen_fds.find(event->getFd());
        // if(it == m_listen_fds.end()){
        //     return;// 没找到就直接退出即可
        // }
        // int op = EPOLL_CTL_DEL;// 默认操作是 删除
        // struct epoll_event tmp_ev = event->getEpollEvent();
        // int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp_ev);
        // if(ret == -1){
        //     ERRORLOG("failed to epoll_ctl when delete fd [%d], error = [%d], error info:[%s]", event->getFd(), errno, strerror(errno));
        // }
        // else DEBUGLOG("delete epoll event success, fd = [%d]", event->getFd());
    } else {
        // if不为当前线程的话，就需要 用到 回调函数
        auto callback = [this, event](){
            DELETE_FROM_EPOLL();
        };
        addTask(callback);
    }
}
EventLoop::~EventLoop(){
    if(m_wakeup_fd_event != nullptr){
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = nullptr;
    }
    // stop();
    close(m_epoll_fd);// 关闭epoll实例对象
}

void EventLoop::dealWakeUp(){

}


} // rocket