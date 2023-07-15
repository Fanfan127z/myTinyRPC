#include "rpc_dispatcher.h"
#include "../codec/tiny_pb_protocol.h"
#include "../../common/log.h"
#include "../../common/error_code.h"
namespace rocket{
// static std::shared_ptr<RpcDispatcher> g_rpc_dispatcher = nullptr;
static RpcDispatcher* g_rpc_dispatcher = nullptr;
// 获取RpcDispatcher 全局单例的对象
// std::shared_ptr<RpcDispatcher> RpcDispatcher::GetRpcDispatcher(){
RpcDispatcher* RpcDispatcher::GetRpcDispatcher(){
    if(g_rpc_dispatcher == nullptr){
        g_rpc_dispatcher = new RpcDispatcher();
    }
    return g_rpc_dispatcher;
}

// 从全名 full_name 中，解析出 service_name 以及 method_name
bool RpcDispatcher::parseServiceFullName(const std::string& full_name
                , std::string& service_name, std::string& method_name){
    if(full_name.empty()){
        ERRORLOG("full_name is empty");
        return false;
    }
    size_t idx = full_name.find_first_of(".");
    if(idx == full_name.npos){
        ERRORLOG("not find '.' in full_name[%s]", full_name.c_str());
        return false;
    }
    service_name = std::string(full_name.begin(), full_name.begin() + idx);
    method_name = std::string(full_name.begin() + idx + 1, full_name.end());
    INFOLOG("success parse service_name[%s], method_name[%s] from full_name[%s]"
            , service_name.c_str(), method_name.c_str(), full_name.c_str());
    return true;
}

// RpcDispatcher类的主要方法就是dispatch
// 输入: request msg, 输出: response msg
/* logic: 
    启动RPC服务时就注册一个 OrderService 对象
        一次RPC服务端调用过程：
        1.先从buffer中，decode得到TinyPbProtocol结构体对象，接着从 请求体 TinyPbProtocol结构体 中得到 method_name，再从 OrderService 对象里根据 service.method_name 找到方法 func
        2.找到对应的 request type 以及 response type
        3.将 请求体 TinyPbProtocol结构体 中的 pb_data 反序列化为 request type 的一个对象，以及 声明一个空的 response type 对象
        4.调用 func(requst, response) 执行 业务逻辑
        5.将 response type 对象 序列化为 pb_data字节流，再塞入到响应的TinyPbProtocol结构体中，最后再 encode 变成字节流转入到 输出out_buffer中，
        接着 注册 可写事件 监听，当可写事件到来触发epoll时，就发送 buffer中的RPC回包 给到客户端 */
void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, const TcpConnection* conn){
    // 先 将 指向 父类协议的指针，转换为 指向特定协议的指针(得强制使用std::dynamic_pointer_cast<PointerAType>来进行转换)
    std::shared_ptr<TinyPbProtocol> req_protocol = std::dynamic_pointer_cast<TinyPbProtocol>(request);
    if(req_protocol == nullptr) { ERRORLOG("convert request msg (std::shared_ptr<AbstractProtocol>) to std::shared_ptr<TinyPbProtocol> error"); return; }
    std::shared_ptr<TinyPbProtocol> response_protocol = std::dynamic_pointer_cast<TinyPbProtocol>(response);
    if(response_protocol == nullptr) { ERRORLOG("convert response msg (std::shared_ptr<AbstractProtocol>) to std::shared_ptr<TinyPbProtocol> error"); return; }
    
    std::string method_full_name = req_protocol->m_method_name;
    // 我们拿到的method名字都是like this 的： service_name.func_name
    // so 要解析一下！
    std::string service_name;
    std::string method_name;
    response_protocol->setMsgId(req_protocol->getMsgId());
    response_protocol->m_method_name = req_protocol->m_method_name;

    // const char* msg_id_c_str = req_protocol->getMsgId().c_str();

    if(parseServiceFullName(method_full_name, service_name, method_name) == false){
        ERRORLOG("msg_id[%s] | parse method_full_name to service_name and method_name error", req_protocol->getMsgId().c_str());
        setTinyPbError(response_protocol, ERROR_PARSE_SERVICE_NAME, "parse service_name error");
        return;
    }
    auto it = m_services_map.find(service_name);
    if(it == m_services_map.end()){
        ERRORLOG("msg_id[%s] | not find service_name[%s]", req_protocol->getMsgId().c_str(), service_name.c_str());
        setTinyPbError(response_protocol, ERROR_SERVICE_NOT_FOUND, "service not find");
        return;
    }
    // 成功拿到service之后，再do真正的事情
    Service_s_ptr service = it->second;
    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if(method == nullptr){
        ERRORLOG("msg_id[%s] | not find method_name[%s] in service_name[%s]", req_protocol->getMsgId().c_str(), method_name.c_str(), service_name.c_str());
        setTinyPbError(response_protocol, ERROR_METHOD_NOT_FOUND, "method not find");
        return;
    }
    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();
    // 反序列化， 将 req_msg.pb_data 反序列化 ;
    // if(req_msg->ParseFromString(req_protocol->m_pb_data) == false){
    if(req_msg->ParseFromString(req_protocol->m_pb_data) == false){
        ERRORLOG("msg_id[%s] | re-serialize error", req_protocol->getMsgId().c_str());
        setTinyPbError(response_protocol, ERROR_FAILED_RE_SERIALIZE, "re-serialize error");
        // 因为是New出来的对象，所以需要及时释放内存
        if(req_msg != nullptr){ delete req_msg; req_msg = nullptr; }
        return;
    }
    // 一旦反序列成功，那么就和 客户端序列化的req_msg是同一个东西
    
    // 打印一下rpc请求的信息
    INFOLOG("msg_id[%s] | get a RPC request[%s]", req_protocol->getMsgId().c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* response_msg = service->GetResponsePrototype(method).New();
    INFOLOG("msg_id[%s] | get response_msg", req_protocol->getMsgId().c_str());
    // 使用service自带的方法，调用 RPC 自定义方法
    // service->CallMethod()的输出参数就是调用RPC方法的响应结果，存放到参数 response_msg 中
    RpcController rpc_controller;
    // TODO: 设置 rpc controller 的控制信息，然后自己理一理思路，防止不知道后面这个有啥用！
    
    rpc_controller.SetPeerAddr(conn->getPeerAddr());
    rpc_controller.SetLocalAddr(conn->getLocalAddr());
    rpc_controller.SetMsgId(req_protocol->getMsgId());
    // 在调用真正的业务方法service->CallMethod()时，通过RpcController对象，拿到这次的RPC上的一些信息
    // 比如：拿到本次RPC调用的本地地址以及远程地址，进而更好地debug
    // TODO:
    INFOLOG("msg_id[%s] | rpc_controller done", req_protocol->getMsgId().c_str());
    service->CallMethod(method, &rpc_controller, req_msg, response_msg, NULL);
    INFOLOG("msg_id[%s] | CallMethod done", req_protocol->getMsgId().c_str());

    // delete rpc_controller; rpc_controller = nullptr;
    // INFOLOG("msg_id[%s] | delete rpc_controller");
    // 将 response_msg的pb_data 做 序列化，得到字节流数据 并 塞入到response_protocol协议的m_pb_data中
    // 此时的，响应的pb_data中，是有正确的响应信息的！
    if(response_msg->SerializeToString(&(response_protocol->m_pb_data)) == false){
        ERRORLOG("msg_id[%s] | serialize error, origin response_msg[%s]", req_protocol->getMsgId().c_str(), response_msg->ShortDebugString().c_str());
        setTinyPbError(response_protocol, ERROR_FAILED_SERIALIZE, "serialize error");
        // 因为是New出来的对象，所以需要及时释放内存
        if(req_msg != nullptr){ delete req_msg; req_msg = nullptr; }
        if(response_msg != nullptr){ delete response_msg; response_msg = nullptr; }
        return;
    }
    // 如果以上代码没有出错的case下，则代表RPC方法 调用成功！
    // 则set错误码是0，代表RPC调用成功！
    response_protocol->m_error_code = 0;
    // 一次RPC调用完成，并打印一下rpc响应的信息
    INFOLOG("msg_id[%s] | Dispatch success(done a RPC Call and got the response), request[%s], response[%s]"
    , req_protocol->getMsgId().c_str(), req_msg->ShortDebugString().c_str(), response_msg->ShortDebugString().c_str());
    // 真正do完这次的RPC调用了，释放对应的request_msg and response_msg
    if(req_msg != nullptr){ delete req_msg; req_msg = nullptr; }
    if(response_msg != nullptr){ delete response_msg; response_msg = nullptr; }
}

// 设置 tinyPbProtocol 协议的错误信息
void RpcDispatcher::setTinyPbError(std::shared_ptr<TinyPbProtocol> msg, int32_t error_code, const std::string& error_info){
    msg->m_error_code = error_code;
    msg->m_error_info = error_info;
    msg->m_error_info_len = error_info.length();
}
// 注册Service对象
void RpcDispatcher::registerService(const Service_s_ptr& service){
    std::string service_name = service->GetDescriptor()->full_name();
    // m_services_map.insert({service_name, service});
    m_services_map[service_name] = service;
}

}// rocket