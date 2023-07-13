#include "tiny_pb_codec.h"
#include "tiny_pb_protocol.h"
#include "../../common/util.h"
#include "../../common/log.h"
#include <vector>
#include <cstring>
#include <string>
#include <arpa/inet.h>


namespace rocket{
// std::shared_ptr<std::string> TinyPbCodec::encodeTinyPb(std::shared_ptr<TinyPbProtocol>& msg, int& msg_len){
const char* TinyPbCodec::encodeTinyPb(std::shared_ptr<TinyPbProtocol>& msg, int& msg_len){
    if(msg->getRequestId().empty()){
        msg->setRequestId("123456789");
    }
    DEBUGLOG("req_id = [%s]", msg->getRequestId().c_str());
    int pk_len = 2 + 24 + msg->getRequestId().length() + msg->m_method_name.length() + msg->m_error_info.length() + msg->m_pb_data.length();
    DEBUGLOG("pk_len = [%d]", pk_len);

    msg_len = pk_len;

    char* tbuf =  reinterpret_cast<char*>(malloc(pk_len));
    char* tmp = tbuf;

    *tmp = TinyPbProtocol::PB_START;
    tmp++;

    int32_t pk_len_net = htonl(pk_len);
    memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
    tmp += sizeof(pk_len_net);
    
    int req_id_len = msg->getRequestId().length();
    int32_t req_id_len_net = htonl(req_id_len);
    memcpy(tmp, &req_id_len_net, sizeof(req_id_len_net));
    tmp += sizeof(req_id_len_net);
    
    std::string req_id_string = msg->getRequestId();
    if(!req_id_string.empty()){// req_id为 非空 时，才do事情！
        const char* req_id = req_id_string.c_str();
        memcpy(tmp, req_id, req_id_len);
        tmp += req_id_len;
    }

    std::string method_name_string = msg->m_method_name;
    int method_name_len = method_name_string.length();
    int32_t method_name_len_net = htonl(method_name_len);
    memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
    tmp += sizeof(method_name_len_net);

    if(!method_name_string.empty()){
        const char* method_name = method_name_string.c_str();
        memcpy(tmp, method_name, method_name_len);
        tmp += method_name_len;
    }

    int32_t error_code = msg->m_error_code;
    int32_t error_code_net = htonl(error_code);
    memcpy(tmp, &error_code_net, sizeof(error_code_net));
    tmp += sizeof(error_code_net);

    int32_t error_info_len = msg->m_error_info.length();
    int32_t error_info_len_net = htonl(error_info_len);
    memcpy(tmp, &error_info_len_net, sizeof(error_info_len_net));
    tmp += sizeof(error_info_len_net);

    std::string error_info_string = msg->m_error_info;
    if(!error_info_string.empty()){
        const char* error_info = error_info_string.c_str();
        memcpy(tmp, error_info, error_info_len);
        tmp += error_info_len;
    }

    int pb_data_len = msg->m_pb_data.length();
    if(!msg->m_pb_data.empty()){
        const char* pb_data = msg->m_pb_data.c_str();
        memcpy(tmp, pb_data, pb_data_len);
        tmp += pb_data_len;
    }

    // int32_t check_sum = msg->m_check_sum;
    int32_t check_sum = 1;// 因为 我现在 还没有写 校验和 的算法，os先赋值一个1
    int32_t check_sum_net = htonl(check_sum);
    memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
    tmp += sizeof(check_sum_net);

    *tmp = TinyPbProtocol::PB_END;

    msg->m_pk_len = pk_len;
    msg->m_req_id_len = req_id_len;
    msg->m_method_name_len = method_name_len;
    msg->m_error_info_len = error_info_len;
    msg->m_parse_success = true;

    // std::string retBuffer = std::string(tbuf);
    // free(tbuf);
    DEBUGLOG("encode msg[%s] success", msg->getRequestId().c_str());
    // return std::make_shared<std::string>(retBuffer);
    return tbuf;
}
// 编码：将 message 对象转化为字节流，写入到 buffer
void TinyPbCodec::encode(std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf){
    for(auto & msg : msgs){
        // 将std::shared_ptr所指向的 父类指针，转换为 对应的子类指针！
        std::shared_ptr<TinyPbProtocol> tmsg = std::dynamic_pointer_cast<TinyPbProtocol>(msg);
        if(!tmsg){// 空的话，就说明没有转换成功（原来的base类指针并没有指向实际的子类对象，就会是返回空）
            ERRORLOG("encode msg transfer error");
        }
        int msg_len = 0;
        // auto msg_str_ptr = encodeTinyPb2(tmsg, msg_len);
        // const char* msg_str = msg_str_ptr->c_str();
        const char* msg_str = encodeTinyPb(tmsg, msg_len);
        if(msg_str != nullptr && msg_len != 0){
            // 写入到 buffer 中
            out_buf->writeToBuffer(msg_str, msg_len);
        } else { DEBUGLOG("failed to write msg to buffer"); }
        if(!msg_str){
            free((void*)msg_str);
            msg_str = nullptr;
        }
    }
}
// 解码：将 buffer 里面的字节流 转换为 message 对象
void TinyPbCodec::decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs){
    // 这个while循环的作用：不断循环调用 内for循环，找到all的完整的RPC包，并解析
    while(true){
        /*  遍历 buffer，找到 PB_START，找到之后，解析出整包的长度，
            然后找到结束符的位置，判断是否是 PB_END，若是，则标识成功得到一个RPC请求 */
        std::vector<char> tmp_buf = buf->m_buffer;
        int start_idx = buf->readIndex();// RPC包的起始位置
        int end_idx = -1;// RPC包的结束位置
        int pk_len = 0;// RPC完整包的长度
        bool parse_success = false;// 判断是否读到一个完整的RPC包
        int i = 0;
        // 这个for循环的作用：是其中找到一个完整的RPC包，并解析
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
        }// for end

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
        // 该 RPC包 已经成功被完整地读取了 
        if(parse_success){
            // 其实 pk_len == end_idx - start_idx + 1
            // buf->moveReadIndex(end_idx - start_idx + 1);
            buf->moveReadIndex(end_idx - start_idx + 1);// 移动可读buffer的index
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
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            msg->m_req_id_len = convertInt32FromNetByte2HostByte(&tmp_buf[req_id_len_idx]);// msg->getRequestId().length();
            DEBUGLOG("parse req_id_len = [%d]", msg->m_req_id_len);
            // 上面求req_id_len的地方我没太搞懂！

            // req_id的索引位置 = req_id_len的起始索引 + sizeof(req_id_len)
            int req_id_idx = req_id_len_idx + sizeof(msg->m_req_id_len);
            if(req_id_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse req_id_idx error, req_id_idx[%d] >= end_idx[%d]", req_id_idx, end_idx);
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
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
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            msg->m_method_name_len = convertInt32FromNetByte2HostByte(&tmp_buf[method_name_len_idx]);
            int method_name_idx = method_name_len_idx + sizeof(msg->m_method_name_len);
            if(method_name_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse method_name_idx error, method_name_idx[%d] >= end_idx[%d]", method_name_idx, end_idx);
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            char method_name[512] {0};
            memcpy(&method_name[0], &tmp_buf[method_name_idx], msg->m_method_name_len);
            msg->m_method_name = std::string(method_name);
            DEBUGLOG("parse method_name success, method_name = [%s]", msg->m_method_name.c_str());

            int error_code_idx = method_name_idx + msg->m_method_name_len;
            if(error_code_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_code_idx error, error_code_idx[%d] >= end_idx[%d]", error_code_idx, end_idx);
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            msg->m_error_code = convertInt32FromNetByte2HostByte(&tmp_buf[error_code_idx]);
            DEBUGLOG("parse error_code success, m_error_code = [%d]", msg->m_error_code);

            int error_info_len_idx = error_code_idx + sizeof(msg->m_error_code);
            if(error_info_len_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_info_len_idx error, error_info_len_idx[%d] >= end_idx[%d]", error_info_len_idx, end_idx);
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            msg->m_error_info_len = convertInt32FromNetByte2HostByte(&tmp_buf[error_info_len_idx]);
            DEBUGLOG("parse error_info_len success, error_info_len = [%d]", msg->m_error_info_len);

            int error_info_idx = error_info_len_idx + sizeof(msg->m_error_info_len);
            if(error_info_idx >= end_idx){
                msg->m_parse_success = false;
                ERRORLOG("parse error_info_idx error, error_info_idx[%d] >= end_idx[%d]", error_info_idx, end_idx);
                continue;// if 解析失败了，就直接跳转到下一次while循环即可
            }
            char error_info[4096] {0};
            memcpy(&error_info[0], &tmp_buf[error_info_idx], msg->m_error_info_len);
            msg->m_error_info = std::string(error_info);
            DEBUGLOG("parse error_info success, error_info = [%s]", msg->m_error_info.c_str());


            int pb_data_len = msg->m_pk_len - msg->m_method_name_len - msg->m_req_id_len - msg->m_error_info_len 
                                            - sizeof(msg->PB_START) - sizeof(msg->PB_END) - 6 * sizeof(int32_t);
            // int pb_data_len = msg->m_pk_len - msg->m_method_name_len - msg->m_req_id_len - msg->m_error_info_len 
            //                                 - 2 - 24;
            
            int pb_data_idx = error_info_idx + msg->m_error_info_len;
            msg->m_pb_data = std::string(&tmp_buf[pb_data_idx], pb_data_len);
            
            // TODO:这里 校验和算法 如何界定这个问题，可自定义 去解析 即可
            msg->m_check_sum = 0;
            // 所有参数 解析成功
            msg->m_parse_success = true;
            // 放入 输出对象队列 中
            out_msgs.push_back(msg);
            DEBUGLOG("decode msg[%s] success", msg->getRequestId().c_str());
        }// if(parse_success)
    }// while
}


}// rocket