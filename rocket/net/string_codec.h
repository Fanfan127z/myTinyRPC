#ifndef ROCKET_NET_STRING_CODEC_H
#define ROCKET_NET_STRING_CODEC_H

#include "./codec/abstract_protocol.h"
#include "./codec/abstract_codec.h"
#include <string>
namespace rocket{
class StringProtocol : public AbstractProtocol{
private:
    std::string m_info;
public:
    StringProtocol() = default;
    StringProtocol(const std::string& info):m_info(info){}
    inline std::string getInfo()const { return m_info; }
    ~StringProtocol() = default;
};
class StringCodec : public AbstractCodec{
public: 
    // 编码：将 message 对象转化为字节流，写入到 buffer
    void encode(const std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf)override;
    // 解码：将 buffer 里面的字节流 转换为 message 对象
    void decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs)override;
};


}// rocket


#endif // ROCKET_NET_STRING_CODEC_H