#ifndef ROCKET_COMMON_MSGID_UTIL_H
#define ROCKET_COMMON_MSGID_UTIL_H

#include <string>
namespace rocket{


class MsgIdUtil{
private:


public:
    MsgIdUtil();
    ~MsgIdUtil();
    // 作用：获取一个随机的数字字符串
    static std::string GenerateMsgId();
};

}// rocket

#endif // ROCKET_COMMON_MSGID_UTIL_H