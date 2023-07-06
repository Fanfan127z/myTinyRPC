#ifndef ROCKET_NET_CODEC_ABSTRACT_CODEC_H
#define ROCKET_NET_CODEC_ABSTRACT_CODEC_H
#include "../tcp/tcp_buffer.h"
#include "abstract_protocol.h"
#include <vector>
#include <memory>
namespace rocket{
// 抽象的编解码器（用于一个RPC msg对象的序列化-把对象转换为字符串）
class AbstractCodec{
private:

public:
    typedef std::shared_ptr<AbstractCodec> s_ptr;
    AbstractCodec() = default;
    // 编码：将 message 对象转化为字节流，写入到 buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf) = 0;
    // 解码：将 buffer 里面的字节流 转换为 message 对象
    virtual void decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs) = 0;

    virtual ~AbstractCodec() = default;// base class 的析构函数 must be virtual 
};

}// rocket
#endif // ROCKET_NET_CODEC_ABSTRACT_CODEC_H