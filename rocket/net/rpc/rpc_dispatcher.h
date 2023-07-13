#ifndef ROCKET_NET_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_DISPATCHER_H
#include <map>
#include <memory>
#include <string>
#include <cstring>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "rpc_controller.h"
#include "../codec/abstract_protocol.h"
#include "../tcp/net_addr.h"
#include "../tcp/tcp_connection.h"
#include "../codec/tiny_pb_protocol.h"
#include "rpc_controller.h"
#include "rpc_closure.h"

namespace rocket{
// 因为TcpConnection类中也引用了rpc_dispachter类，因此造成了循环引用的问题！
// TODO: 循环引用问题需要 使用 类的前置声明来解决！
class TcpConnection;
class RpcDispatcher{
public:
    typedef std::shared_ptr<google::protobuf::Service> Service_s_ptr;
    typedef std::shared_ptr<RpcDispatcher> s_ptr;
private:
    // < key, val > == < service_name, Google_Protobuf_Service_object_ptr >
    std::map<std::string, Service_s_ptr> m_services_map;// 存放服务名称 + 指向具体服务的智能指针对象
    // std::shared_ptr<RpcController> m_rpc_controller;
public:
    // RpcDispatcher() = default;
    // ~RpcDispatcher() = default;
    // RpcDispatcher类的主要方法就是dispatch
    // 输入: request msg, 输出: response msg
    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, const TcpConnection* conn);
    // 注册Service对象
    void registerService(const Service_s_ptr& service);
    // 设置 tinyPbProtocol 协议的错误信息
    void setTinyPbError(std::shared_ptr<TinyPbProtocol> msg, int32_t error_code, const std::string& error_info);
    
    // static std::shared_ptr<RpcDispatcher> GetRpcDispatcher();// 获取RpcDispatcher 全局单例的对象
    static RpcDispatcher* GetRpcDispatcher();// 获取RpcDispatcher 全局单例的对象
    
private:
    // 从全名 full_name 中，解析出 service_name 以及 method_name
    bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);
};

}// rocket

#endif // ROCKET_NET_RPC_RPC_DISPATCHER_H