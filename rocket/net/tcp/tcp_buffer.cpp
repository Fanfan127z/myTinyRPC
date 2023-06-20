#include "tcp_buffer.h"
#include "../../common/log.h"
#include <string.h>

namespace rocket{

TcpBuffer::TcpBuffer(size_t size):m_size(size) {
    m_buffer.resize(m_size);
    DEBUGLOG("success create TcpBuffer, which size is [%d]", m_size);
}

// 返回可读的字节数
int TcpBuffer::readAble() const {
    return m_writeIndex - m_readIndex;
}
// 返回可写的字节数
int TcpBuffer::writeAble() const {
    return m_size - m_writeIndex;
}
 
// 向buffer中写入数据(写入buf，大小是size)
void TcpBuffer::writeToBuffer(const char* buf, size_t size){
    // 主要逻辑：只要buff中还有可写空间，就继续写，否则，先覆盖掉已经读完的idxs，再继续写
    
    // 调整buffer的大小，扩容到合理为止
    while(true){
        if( (int)size > writeAble() ){ // 此时没有可写空间，先扩容且搬移，再重新写
            resizeBuffer(m_size * 2);// 直接2倍扩容即可
        } else {
            break;
        }
    }
    // 此时可写，写就完事了
    ScopeMutex<Mutex> lock(m_mutex);
    /*  if 不用memcpy()函数的话可以直接这么写，都是一样的！
        int idx = 0;
        while(idx < size){
            m_buffer[m_writeIndex++] = buf[idx++];
        }
    */
    memcpy(&m_buffer[m_writeIndex], buf, size);
    m_writeIndex += size;// 千万别忘记这个！
    lock.unlock();
    DEBUGLOG("success write buf[%s] to TcpBuffer", buf);
}
 // 给buffer扩容
void TcpBuffer::resizeBuffer(size_t new_size){

    // int real_new_size = max(new_size, readAble());
    std::vector<char> tmp_buffer(new_size);

    ScopeMutex<Mutex> lock(m_mutex);
    m_size = new_size;// 更新buffer大小, very important
    int startIdx = 0;
    // 先搬移剩余的没有被读的空间搬移到到临时tmp_buffer的前面
    while(m_readIndex < m_writeIndex){
        tmp_buffer[startIdx++] = m_buffer[m_readIndex++];
    }
    int new_readIndex = 0;
    int new_writeIndex = startIdx;

    m_buffer.swap(tmp_buffer);
    m_readIndex = new_readIndex;
    m_writeIndex = new_writeIndex;
    lock.unlock();
    DEBUGLOG("success resize TcpBuffer, which new length is [%d]", new_size);
}
// 从buffer中读取数据(读入到re中, res是传入传出参数)
void TcpBuffer::readFromBuffer(std::vector<char>& re, size_t size){
    // 主要逻辑：只要buff中还有可读空间，就继续读，否则不读了
    if(readAble() <= 0){
        INFOLOG("Failed to read bytes from TcpBuffer, current TcpBuffer is un_readAble");
        return;// 没有可读的buffer字节
    }
    /*
        if 当前指定要读取的字节数size大于当前buffer中可读的个数，就只读取可读的部分即可
        否则，继续读size个buffer中的data
    */
    int real_read_size = (int)size > readAble() ? readAble() : size;
    re.resize(real_read_size);
    ScopeMutex<Mutex> lock(m_mutex);
    int idx = 0;
    while(real_read_size--){
        re[idx++] = m_buffer[m_readIndex++];
    }
    // 每一次read完成之后，都do一次调整buffer的操作，以保证buffer不太大也不太小
    adjustBuffer();
    INFOLOG("success read [%s] from TcpBuffer");
}

/* 
    调整buffer，让buffer不需要频繁的扩容，提升buffer的使用性能
    (很多时候，调整了就可以避免频繁的扩容resizeBuffer操作，但是具体还得看应用这个buffer的场景如何)
*/
void TcpBuffer::adjustBuffer(){

    size_t threshold_for_adjust_buffer = m_size * 2 / 3;// 当已被读取的index超出总大小的2/3时，说明此时需要调整一下了！
    // 超过 调整buffer阈值的时候，需要动态调整数组（底层做的事搬移的工作）！
    bool needToAdjust = ( m_readIndex <= threshold_for_adjust_buffer ) ? false : true;
    if(!needToAdjust){
        DEBUGLOG("no need to adjust TcpBuffer");
        return;
    }
    std::vector<char> tmp_buffer(m_size);
    ScopeMutex<Mutex> lock(m_mutex);
    int startIdx = 0;
    // 先搬移剩余的没有被读的空间搬移到到临时tmp_buffer的前面
    while(m_readIndex < m_writeIndex){
        tmp_buffer[startIdx++] = m_buffer[m_readIndex++];
    }
    int new_readIndex = 0;
    int new_writeIndex = startIdx;

    m_buffer.swap(tmp_buffer);
    m_readIndex = new_readIndex;
    m_writeIndex = new_writeIndex;
    lock.unlock();
    DEBUGLOG("success adjust TcpBuffer");
}


void TcpBuffer::moveReadIndex(size_t size){
    size_t new_readIdx = m_readIndex + size;
    if(new_readIdx >= m_size){
        ERRORLOG("moveReadIndex error, invalid size[%d], old_read_idx[%d], buffer size[%d]", size, m_readIndex, m_size);
        return;
    }
    m_readIndex = new_readIdx;
    adjustBuffer();// 每一次move可读index之后就必须要调整一下buffer
}
void TcpBuffer::moveWriteIndex(size_t size){
    size_t new_writeIdx = m_writeIndex + size;
    if(new_writeIdx >= m_size){
        ERRORLOG("moveWriteIndex error, invalid size[%d], old_read_idx[%d], buffer size[%d]", size, m_readIndex, m_size);
        return;
    }
    m_readIndex = new_writeIdx;
}



TcpBuffer::~TcpBuffer(){

}

}// rocket