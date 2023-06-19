#ifndef ROCKET_NET_TCP_BUFFER_H
#define ROCKET_NET_TCP_BUFFER_H

// #include <list> 
#include <vector> // 因为buffer是频繁增删的，其实用list更好
#include "../../common/mutex.h"
namespace rocket{

/*
        readIndex         writeIndex
            |                 |
buf:       a b c A C D F R E    
size: 10
buf idx:   0 1 2 3 4 5 6 7 8 9
        ==> 可读字节数 == writeIndex - readIndex;
        ==> 可写字节数 == m_size - writeIndex;
*/
class TcpBuffer{
private:
    // [readIndex, writeIndex) use 左闭右开区间
    int m_readIndex {0};// buffer的可读头下标
    int m_writeIndex {0};// buffer的可写尾下标
    size_t m_size {0};// 大小
    std::vector<char> m_buffer;// buffer
    Mutex m_mutex;
public:
    TcpBuffer(size_t size);

    // 返回可读的字节数
    int readAble() const;
    // 返回可写的字节数
    int writeAble() const;
    
    inline int readIndex() const { return m_readIndex; }
    inline int writeIndex() const { return m_writeIndex; }

    // 向buffer中写入数据(写入buf，大小是size)
    void writeToBuffer(const char* buf, size_t size);

    // 从buffer中读取数据(读入到re中)
    void readFromBuffer(std::vector<char>& re, size_t size);

    // 给buffer扩容
    void resizeBuffer(size_t new_size);
    /* 
        调整buffer，让buffer不需要频繁的扩容，提升buffer的使用性能
        (很多时候，调整了就可以避免频繁的扩容resizeBuffer操作，但是具体还得看应用这个buffer的场景如何)
    */
    void adjustBuffer();

    // 下面这2个function其实，我觉得没必要存在，但还是先写一下，万一以后有别的想法要改造这个buffer呢？
    void moveReadIndex(size_t size);
    void moveWriteIndex(size_t size);

    ~TcpBuffer();
};





}// rocket

#endif // ROCKET_NET_TCP_BUFFER_H