#ifndef ROCKET_NET_CODEC_TINY_PB_PROTOCOL_H
#define ROCKET_NET_CODEC_TINY_PB_PROTOCOL_H

#include "abstract_protocol.h"
#include <string>
namespace rocket{

struct TinyPbProtocol : public AbstractProtocol{
public:
    // 用static（全局，不受运行时构造的任何对象的类型的影响）
    // 标识这个开始和结束符是静态的，说明all对象的这两个值都是一毛一样的！
    static char PB_START;// 包的开始符
    static char PB_END;// 包的结束符
public:
    int32_t m_pk_len {0};
    int32_t m_req_id_len {0};
    // m_req_id继承自父类
    int32_t m_method_name_len {0};
    std::string m_method_name;
    int32_t m_error_code {0};
    int32_t m_error_info_len {0};
    std::string m_error_info;
    std::string m_pb_data;
    int32_t m_check_sum {0};
    bool m_parse_success {false};// 代表该msg是否是一个解析成功的包
public:
    TinyPbProtocol();
    
    ~TinyPbProtocol();
};

char TinyPbProtocol::PB_START = 0x02;
char TinyPbProtocol::PB_END = 0x03;

}// rocket

#endif // ROCKET_NET_CODEC_TINY_PB_PROTOCOL_H