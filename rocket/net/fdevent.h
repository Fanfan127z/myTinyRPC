#ifndef ROCKET_NET_FDEVENT_H
#define ROCKET_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>
namespace rocket {

class FdEvent{
public:
    enum TriggerEvent{
        IN_EVENT = EPOLLIN, // 可读事件 触发标志
        OUT_EVENT = EPOLLOUT, // 可写事件 触发标志
    };
    FdEvent();
    FdEvent(int fd);
    // FdEvent(TriggerEvent event_type);
    std::function<void()> handler(TriggerEvent event_type);
    // 监听
    void listen(TriggerEvent event_type, const std::function<void()>& callback);
    // 取消监听
    void cancle_listen(TriggerEvent event_type);
    void setNonBlock();// set该fd是非阻塞的
    inline int getFd() const { return m_fd; }
    inline struct epoll_event getEpollEvent(){ return m_listen_event; }
    virtual ~FdEvent();// 被用做 基类 的类，析构函数必须是virtual的！

protected:
// private:
    int m_fd {-1};
    struct epoll_event m_listen_event;         // 监听的事件
    std::function<void()> m_read_callback;      // 读回调函数
    std::function<void()> m_write_callback;     // 写回调函数
};

}

#endif // ROCKET_NET_FDEVENT_H