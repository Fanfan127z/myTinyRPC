#ifndef ROCKET_NET_CODEC_TINY_PB_CODEC_H
#define ROCKET_NET_CODEC_TINY_PB_CODEC_H

#include "abstract_codec.h"
#include "tiny_pb_protocol.h"
#include <memory>

namespace rocket{

class TinyPbCodec : public AbstractCodec{
private:
    // 输入msg,传入传出参数msg_len,最终得到转换成功的字节流数据(const char*)
    // TODO: 这么do是有问题的！具体问题我还没有find出来
    // std::shared_ptr<std::string>  encodeTinyPb2(std::shared_ptr<TinyPbProtocol>& msg, int& msg_len);
    const char* encodeTinyPb(std::shared_ptr<TinyPbProtocol>& msg, int& msg_len);
public:
    TinyPbCodec() = default;
    // 编码：将 message 对象转化为字节流，写入到 buffer
    void encode(std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf)override;
    // 解码：将 buffer 里面的字节流 转换为 message 对象
    void decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs)override;

    ~TinyPbCodec() = default;
};

}// rocket

#endif // ROCKET_NET_CODEC_TINY_PB_CODEC_H