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
    void listen(TriggerEvent event_type, const std::function<void()>& callback);
    
    inline int getFd() const { return m_fd; }
    inline struct epoll_event getEpollEvent(){ return m_listen_event; }
    ~FdEvent();

protected:
// private:
    int m_fd {-1};
    struct epoll_event m_listen_event;         // 监听的事件
    std::function<void()> m_read_callback;      // 读回调函数
    std::function<void()> m_write_callback;     // 写回调函数
};

}

#endif // ROCKET_NET_FDEVENT_H