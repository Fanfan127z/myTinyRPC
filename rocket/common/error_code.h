#ifndef ROCKET_COMMON_ERROR_CODE_H
#define ROCKET_COMMON_ERROR_CODE_H

#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif
/*
    注意，错误码都是自定义的，只需要你通讯双方客户端服务端都认同
    (就是遇到同一个错误码做同样的事情的意思)这同一份的错误码，那么你写的服务就能够很好的work
*/ 
/*
namespace rocket{
}// rocket
*/
const int ERROR_PEER_CLOSED = SYS_ERROR_PREFIX(0000);// 建立好连接之后，对端关闭了
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001); // 简历TCP连接失败
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002);// 获取 回包 失败
const int ERROR_FAILED_RE_SERIALIZE = SYS_ERROR_PREFIX(0003);// 反序列化 失败
const int ERROR_FAILED_SERIALIZE = SYS_ERROR_PREFIX(0004);// 序列化 失败

const int ERROR_FAILED_ENCODE = SYS_ERROR_PREFIX(0005);// encode 失败
const int ERROR_FAILED_DECODE = SYS_ERROR_PREFIX(0006);// decode 失败
const int ERROR_RPC_CALL_TIMEOUT = SYS_ERROR_PREFIX(0007);// RPC 调用超时

const int ERROR_SERVICE_NOT_FOUND = SYS_ERROR_PREFIX(0008);// 找不到service，即service不存在的意思 
const int ERROR_METHOD_NOT_FOUND = SYS_ERROR_PREFIX(0009);// 找不到method，即method不存在的意思

const int ERROR_PARSE_SERVICE_NAME = SYS_ERROR_PREFIX(0010);// 解析service_name失败

#endif // ROCKET_COMMON_ERROR_CODE_H