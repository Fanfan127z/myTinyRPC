#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H
#include <google/protobuf/service.h>
#include "../tcp/tcp_client.h"
#include "rpc_closure.h"
#include "../tcp/net_addr.h"
#include "../timer_event.h"// use 定时器
#include "rpc_controller.h"
#include <memory>

namespace rocket{
/* RpcChannel
    作用：用于Rpc客户端与服务端进行通信(客户端发起RPC连接)的意思！

            (request)                                                (response)
    [connect] ----> [encode] ----> [write] ----> [read] ----> [decode]
    rpc_channel把客户端连接服务器端，encode，write，read，decode等实现都封装起来，直接调用rpc_channel即可认为是发起了一个RPC请求，从而屏蔽了底层的细节！
    一次RPC通信所需要的procedures：
    1: client 连接 对端服务器端
    2: 构造真正的request请求体 并 encode成 protocol_msg(我们自定义的协议格式的信息)
    3：将msg发送到服务器端
    4：读取服务器端给我当前client的回包
    5：将protocol_msg decode解析成 真正的response响应体

    以上5个步骤就是完成一次RPC通信所需要的步骤。
*/
// TODO: 封装为宏定义的作用是：不用暴露我代码中使用到shared_ptr来管理request和response的细节，防止调用方没有使用智能指针导致内存的泄露问题！
// 同理：封装channel和controller的宏定义也是如此！

#define NEWMESSAGE(type, var_name)\
    std::shared_ptr<type> var_name = std::make_shared<type>();\

#define NEWRPCCONTROLLER(var_name)\
    std::shared_ptr<rocket::RpcController> var_name = std::make_shared<rocket::RpcController>();\

#define NEWRPCCHANNEL(peer_addr, var_name)\
    std::shared_ptr<rocket::RpcChannel> var_name = std::make_shared<rocket::RpcChannel>(std::make_shared<rocket::IPv4NetAddr>(peer_addr));\


// CALLRPC的过程：
// 1:先根据对端地址，创建channel
// 2:再初始化channel all的信息
// 3:再创建Order_Stub对象or临时对象（然后才能调用RPC方法） 
// 4:最后调用RPC method
// 注：必须在使用宏定义 NEWRPCCHANNEL 创建了channel之后再使用宏定义 CALLRPC 调用RPC服务

// TODO: 实际上，stub.makeOrder()内部会调用到channel->CallMethod(controller, request, response, closure)方法
// 下面 Order_Stub(channel.get());是构造一个临时的Order_Stub对象，和你 Order_Stub stub(channel.get());创建的对象再去调用callmethod没什么区别


#define CALLRPC(method_name, controller, request, response, closure)\
    {\
        channel->Init(controller, request, response, closure);\
        Order_Stub(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get());\
    }\

/*   std::enable_shared_from_this<>是C++标准库中的一个模板类。通过继承这个类，并在类的实例中使用std::shared_ptr来管理对象的生命周期，可以获得对象自身的std::shared_ptr指针。
    这样的话，在需要使用对象的std::shared_ptr指针时，可以通过调用shared_from_this()成员函数来获得该指针
    而且在该类的外部也需要使用shared_ptr来管理RpcChannel对象，否则会报错 */
class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>{
public:
    typedef std::shared_ptr<RpcChannel> s_ptr;
    typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
    typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
    typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;
private:
    NetAddrBase::s_ptr m_local_addr {nullptr};// 调用RPC服务端本地地址
    NetAddrBase::s_ptr m_peer_addr {nullptr};// 对端地址
    // 为了防止，在回调函数中，我们的传入指针参数 controller request response done 这四个指针被释放掉，应该使用shared_ptr来do函数传参这个事情！
    controller_s_ptr m_controller {nullptr};// RPC控制器
    message_s_ptr m_request {nullptr};// RPC请求
    message_s_ptr m_response {nullptr};// RPC响应
    closure_s_ptr m_closure {nullptr};// RPC调用成功之后需要执行的回调函数

    bool m_is_init {false};// 代表是否初始化RpcChannel成功
    TcpClient::s_ptr m_client {nullptr};// 代表当前发起RPC请求的客户端
    TimerEvent::s_ptr m_timer_event {nullptr};// 超时定时器任务事件
public:
    // 注意：当RpcChannel类对象析构的时候，这些对象也会被一一析构掉！（因为是智能指针管理heap内存）
    inline controller_s_ptr GetController() const { return m_controller; }
    inline message_s_ptr GetRequest() const { return m_request; }
    inline message_s_ptr GetResponse() const { return m_response; }
    inline closure_s_ptr GetClosure() const { return m_closure; }
    inline TcpClient::s_ptr GetTcpClient() const { return m_client; }
    inline TimerEvent::s_ptr GetTimerEvent() const { return m_timer_event; }
public:
    RpcChannel(const NetAddrBase::s_ptr& peer_addr);
    ~RpcChannel();
    void Init(controller_s_ptr controller, message_s_ptr request, message_s_ptr response, closure_s_ptr done);
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) ;
    void CallBack();
};


}// rocket

#endif // ROCKET_NET_RPC_RPC_CHANNEL_H