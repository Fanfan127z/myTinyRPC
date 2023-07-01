#ifndef ROCKET_NET_CODEC_TINY_PB_CODEC_H
#define ROCKET_NET_CODEC_TINY_PB_CODEC_H

#include "abstract_codec.h"

namespace rocket{

class TinyPbCodec : public AbstractCodec{
private:

public:
    TinyPbCodec();
    // 编码：将 message 对象转化为字节流，写入到 buffer
    void encode(const std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf);
    // 解码：将 buffer 里面的字节流 转换为 message 对象
    void decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs);

    ~TinyPbCodec();
};

}// rocket

#endif // ROCKET_NET_CODEC_TINY_PB_CODEC_H