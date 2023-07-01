#include "tiny_pb_codec.h"
#include "tiny_pb_protocol.h"
#include "../../common/util.h"
#include "../../common/log.h"
#include <vector>
#include <cstring>

namespace rocket{

 // 编码：将 message 对象转化为字节流，写入到 buffer
void TinyPbCodec::encode(const std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf){
    
}
// 解码：将 buffer 里面的字节流 转换为 message 对象
void TinyPbCodec::decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs){
    
    while(true){
        /*  遍历 buffer，找到 PB_START，找到之后，解析出整包的长度，
            然后找到结束符的位置，判断是否是 PB_END，若是，则标识成功得到一个RPC请求 */
        std::vector<char> tmp_buf = buf->m_buffer;
        int start_idx = buf->readIndex();// RPC包的起始位置
        int end_idx = -1;// RPC包的结束位置
        int pk_len = 0;// RPC完整包的长度
        bool parse_success = false;// 判断是否读到一个完整的RPC包
        int i = 0;
        for(i = start_idx;i < buf->writeIndex();++i){
            if(tmp_buf[i] == TinyPbProtocol::PB_START){
                // 读下去的四个字节，由于是网络字节序，so需要转换为主机字节序
                if(i + 1 < buf->writeIndex()){// 判断是否已经越界（buffer的可读范围是[readIndex, writeIdx)）了
                    pk_len = convertInt32FromNetByte2HostByte(&tmp_buf[i+1]);
                    DEBUGLOG("get rpc-package len:[%d]", pk_len);
                    // 结束符的索引
                    int t_end_idx = i + pk_len - 1;
                    if(t_end_idx >= buf->writeIndex()){
                        continue;// 此时说明没有读到一个完整的RPC包，就continue就绪
                    }
                    if(tmp_buf[t_end_idx] == TinyPbProtocol::PB_END){
                        start_idx = i;
                        end_idx = t_end_idx;
                        parse_success = true;
                        break;// 跳出循环
                    }
                }
            }
        }// for
        if(i >= buf->writeIndex()){// 此时，buffer中可读可写的index都 遍历完毕了！
            DEBUGLOG("TinyPbCodec::decode end, read all buffer data");
            break;
        }
        /*
                readIdx                 writeIdx
                    |                        |
        index:      0    1    2    3    4    5   ... 
        buffer: ['0x02', x1, x2, x3, '0x03', xn, ...]
        in this case:
                |                   |
                start_idx           end_idx
        */
        // 该RPC包已经成功被读取 
        if(parse_success){
            // 其实 pk_len == end_idx - start_idx + 1
            // buf->moveReadIndex(end_idx - start_idx + 1);
            buf->moveReadIndex(pk_len);// 移动可读buffer的index
            /* 构造协议对象 的成员变量如下：
               int32_t m_pk_len int32_t m_req_id_len m_req_id继承自父类 int32_t m_method_name_len std::string m_method_name
               int32_t m_error_code int32_t m_error_info_len std::string m_error_info std::string m_pb_data int32_t m_check_sum */
            std::shared_ptr<TinyPbProtocol> msg = std::make_shared<TinyPbProtocol>();
            msg->m_pk_len = pk_len;
            // req_id_len的索引位置 = 当前完整的RPC包的起始索引 + sizeof(包开始符) + sizeof(整个包的长度)
            int req_id_len_idx = start_idx + sizeof(TinyPbProtocol::PB_START) + sizeof(msg->m_pk_len);
            if(req_id_len_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse req_id_len_idx error, req_id_len_idx[%d] >= end_idx[%d]", req_id_len_idx, end_idx);
            }
            msg->m_req_id_len = convertInt32FromNetByte2HostByte(&tmp_buf[req_id_len_idx]);// msg->getRequestId().length();
            DEBUGLOG("parse req_id_len = [%d]", msg->m_req_id_len);
            // 上面求req_id_len的地方我没太搞懂！
            // req_id的索引位置 = req_id_len的起始索引 + sizeof(req_id_len)
            int req_id_idx = req_id_len_idx + sizeof(msg->m_req_id_len);
            if(req_id_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse req_id_idx error, req_id_idx[%d] >= end_idx[%d]", req_id_idx, end_idx);
            }
            char req_id[128] {0};
            // 从tmp_buf[req_id_idx]位置开始读取, msg->m_req_id_len个字节 并赋值给 req_id从idx==0开始的位置
            memcpy(&req_id[0], &tmp_buf[req_id_idx], msg->m_req_id_len);
            msg->setRequestId(std::string(req_id));
            DEBUGLOG("parse req_id success, req_id = [%s]", msg->getRequestId().c_str());

            int method_name_len_idx = req_id_idx + msg->m_req_id_len;
            if(method_name_len_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse method_name_len_idx error, method_name_len_idx[%d] >= end_idx[%d]", method_name_len_idx, end_idx);
            }
            msg->m_method_name_len = convertInt32FromNetByte2HostByte(&tmp_buf[method_name_len_idx]);
            int method_name_idx = method_name_len_idx + sizeof(msg->m_method_name_len);
            if(method_name_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse method_name_idx error, method_name_idx[%d] >= end_idx[%d]", method_name_idx, end_idx);
            }
            char method_name[512] {0};
            memcpy(&method_name[0], &tmp_buf[method_name_idx], msg->m_method_name_len);
            msg->m_method_name = std::string(method_name);
            DEBUGLOG("parse method_name success, method_name = [%s]", msg->m_method_name.c_str());

            int error_code_idx = method_name_idx + msg->m_method_name_len;
            if(error_code_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_code_idx error, error_code_idx[%d] >= end_idx[%d]", error_code_idx, end_idx);
            }
            msg->m_error_code = convertInt32FromNetByte2HostByte(&tmp_buf[error_code_idx]);
            DEBUGLOG("parse error_code success, m_error_code = [%d]", msg->m_error_code);

            int error_info_len_idx = error_code_idx + sizeof(msg->m_error_code);
            if(error_info_len_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_info_len_idx error, error_info_len_idx[%d] >= end_idx[%d]", error_info_len_idx, end_idx);
            }
            msg->m_error_info_len = convertInt32FromNetByte2HostByte(&tmp_buf[error_info_len_idx]);
            DEBUGLOG("parse error_info_len success, error_info_len = [%d]", msg->m_error_info_len);
            int error_info_idx = error_info_len_idx + sizeof(msg->m_error_info_len);
            if(error_info_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_info_idx error, error_info_idx[%d] >= end_idx[%d]", error_info_idx, end_idx);
            }
            char error_info[4096] {0};
            memcpy(&error_info[0], &tmp_buf[error_info_idx], msg->m_error_info_len);
            msg->m_error_info = std::string(error_info);
            DEBUGLOG("parse error_info success, error_info = [%s]", msg->m_error_info.c_str());


            int pb_data_len = msg->m_pk_len - msg->m_method_name_len - msg->m_req_id_len - msg->m_error_info_len 
                                            - sizeof(msg->PB_START) - sizeof(msg->PB_END) - 6 * sizeof(int32_t);
            
            int pb_data_idx = error_info_idx + msg->m_error_info_len;
            msg->m_pb_data = std::string(&tmp_buf[pb_data_idx], pb_data_len);
            
            // TODO:这里 校验和 自定义 去解析 即可
            msg->m_check_sum = 0;
            // 所有参数 解析成功
            msg->m_parse_success = true;
            // 放入 输出对象队列 中
            out_msgs.emplace_back(msg);
        }// if(parse_success)
    }
}


}// rocket