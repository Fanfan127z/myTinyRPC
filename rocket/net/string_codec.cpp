#include "string_codec.h"
#include "../common/log.h"
#include <error.h>
#include <cstring>
namespace rocket{

// 编码：将 message 对象转化为字节流，写入到 buffer
void StringCodec::encode(std::vector<AbstractProtocol::s_ptr>& msgs, TcpBuffer::s_ptr& out_buf){
    for(size_t i = 0;i < msgs.size(); ++i){
        /* dynamic_pointer_cast是C++中的一种智能指针类型转换操作符，用于在类层次结构中进行安全的向下转型。
          它与dynamic_cast类似，但是用于智能指针而不是裸指针。*/
        std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(msgs[i]);
        if(!msg) { DEBUGLOG("dynamic_cast( StringProtocol* ) error, errno = [%d], error info = [%s]", errno, strerror(errno)); }
        
        out_buf->writeToBuffer(msg->getInfo().c_str(), msg->getInfo().length());
    }
}
// 解码：将 buffer 里面的字节流 转换为 message 对象
void StringCodec::decode(TcpBuffer::s_ptr& buf, std::vector<AbstractProtocol::s_ptr>& out_msgs){
    std::vector<char> tmp_re;
    buf->readFromBuffer(tmp_re, buf->readAble());
    std::string info(tmp_re.begin(), tmp_re.end());// convert std::vector<char> 2 std::string
    auto msg_ptr = std::make_shared<StringProtocol>(info);
    msg_ptr->setMsgId("123456");// 暂时写死
    out_msgs.push_back(msg_ptr);
}


}// rocket