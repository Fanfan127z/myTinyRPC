#ifndef ROCKET_COMMON_MUTEX_H
#define ROCKET_COMMON_MUTEX_H
#include <pthread.h>
// #include <assert.h>
// #include <iostream>

// 类似于C++11引入的std::lock_guard一样！避免了手动管理锁的复杂性，因此可以减少死锁和资源泄漏的风险
template<class T>
class ScopeMutex{
private:
    T m_mutex;
    bool m_is_lock {false};
public:
    explicit ScopeMutex(const T& mutex):m_mutex(mutex){
        m_is_lock = {true};// 构造时就顺便加锁
        m_mutex.lock();
    }
    inline void lock(){
        if(!m_is_lock){// 没有加锁时，才进行加锁操作
            m_mutex.lock();
        }
    }
    inline void unlock(){
        if(m_is_lock){// 加锁时，才进行解锁操作
            m_mutex.unlock();
        }
    }
    ~ScopeMutex(){
        m_is_lock = {false};// 析构时就顺便解锁
        m_mutex.unlock();
    }
};
class Mutex{
private:
    pthread_mutex_t m_mutex;
public:
    Mutex(){
        pthread_mutex_init(&m_mutex, nullptr);
    }
    inline void lock(){
        // assert(pthread_mutex_lock(&m_mutex) == 0);// 我觉得，对于一个高可维护性的程序，不容易出error的程序，不要那么严格地出一点错就那啥！
        pthread_mutex_lock(&m_mutex);
    }
    inline void unlock(){
        // assert(pthread_mutex_unlock(&m_mutex) == 0);
        pthread_mutex_unlock(&m_mutex);
    }
    inline pthread_mutex_t* getRawMutex() { return &m_mutex; }
    ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
        
    }
};

#endif // ROCKET_COMMON_MUTEX_H