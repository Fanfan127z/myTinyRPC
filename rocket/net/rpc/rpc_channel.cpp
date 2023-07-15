#include <memory>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpc_channel.h"
#include "../../common/log.h"
#include "../codec/tiny_pb_protocol.h"
#include "../../common/msg_id_util.h"
#include "rpc_controller.h"
#include "../../common/error_code.h"

namespace rocket{
void RpcChannel::CallBack(){
    RpcController* my_rpc_controller = dynamic_cast<RpcController*>(GetController().get());
    if(my_rpc_controller->IsFinished()){
        return;
    }
    if(m_closure){
        m_closure->Run();
        if(my_rpc_controller){
            my_rpc_controller->SetFinished(true);// 设置本次RPC已经完成
        }
        INFOLOG("RpcChannel::CallBack() done");
    }
}
RpcChannel::RpcChannel(const NetAddrBase::s_ptr& peer_addr) : m_peer_addr(peer_addr){
    DEBUGLOG("RpcChannel()");
    m_client = std::make_shared<TcpClient>(m_peer_addr);// 创建tcp m_client
}
RpcChannel::~RpcChannel(){
    DEBUGLOG("~RpcChannel()");
}
// 这样就能一直保存这个RpcChannel对象的智能指针，不会被中途调用各种深层的回调函数（比如m_client.Write() or .Read()时被释放掉）
void RpcChannel::Init(controller_s_ptr controller, message_s_ptr request, message_s_ptr response, closure_s_ptr done){
    if(m_is_init)return;// 无需 重复 初始化
    m_controller = controller;
    m_request = request;
    m_response = response;
    m_closure = done;

    m_is_init = true;
}
// 1：构建新的 request_protocol对象，并赋值MsgID，method_name 以及 pb_data(真正的数据)
// 2：从客户端发送到服务器端，并接受回包，如果能够接受回包成功，则说明本次RPC调用成功，执行done函数
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
        google::protobuf::Message* response, google::protobuf::Closure* done){
    /*
        在C++中，dynamic_cast主要用于处理多态性。当我们需要在继承层次结构中进行向下转型（即从基类向派生类转型）时，
        通常会使用dynamic_cast。dynamic_cast会在运行时检查转换是否安全。如果转换是安全的，那么转换就会成功；
        如果转换不安全，那么转换就会失败，dynamic_cast会返回一个空指针。
    */
    RpcController* my_rpc_controller = dynamic_cast<RpcController*>(controller);
    if(my_rpc_controller == nullptr){ 
        ERRORLOG("RpcController dynamic_cast convert error");
        my_rpc_controller->SetError(ERROR_RPC_CONTROLLER, "RpcController dynamic_cast convert error");// 待完善
        CallBack();
        return;
    }
    if(m_peer_addr == nullptr){
        ERRORLOG("failed to get peer addr");
        my_rpc_controller->SetError(ERROR_RPC_PEER_ADDR, "peer addr is nullptr");
        CallBack();
        return;
    }
    std::shared_ptr<TinyPbProtocol> req_protocol = std::make_shared<TinyPbProtocol>();// 要发送的自定义协议msg
    if(my_rpc_controller->GetMsgId().empty()){
        req_protocol->setMsgId(MsgIdUtil::GenerateMsgId());// if controller中是空的MsgId的话，就重新生成一个
        my_rpc_controller->SetMsgId(req_protocol->getMsgId());// 同时，将MsgId设置进去controller中
    } else {
        req_protocol->setMsgId(my_rpc_controller->GetMsgId());// if 不为空的话，就直接set为controller中的MsgId号即可了！
    }
    req_protocol->m_method_name = method->full_name();
    INFOLOG("MsgId[%s] | call method name[%s]", req_protocol->getMsgId().c_str(), req_protocol->m_method_name.c_str());
    
    if(!m_is_init){
        std::string error_info = "RpcChannel do not be inited";
        my_rpc_controller->SetError(ERROR_RPC_CHANNEL_INIT, error_info);
        ERRORLOG("msg_id[%s] | RpcChannel do not be inited", req_protocol->getMsgId().c_str());
        CallBack();
        return;
    }
    // do request 的 序列化
    if(request->SerializeToString(&(req_protocol->m_pb_data)) == false){
        std::string error_info = "failed to serialize";
        my_rpc_controller->SetError(ERROR_FAILED_SERIALIZE, error_info);
        ERRORLOG("msg_id[%s] | %s, origin request[%s]", req_protocol->getMsgId().c_str(), error_info.c_str(), request->ShortDebugString().c_str());
        CallBack();
        return;
    }
    /* TODO: 说明：
        google::protobuf::Closure* done 这个函数的含义是：当RPC调用成功时，就会执行这个函数！
       当m_client收到RPC回包时，就说明这次的RPC请求是成功调用的！此时就可以去执行 done函数 所要执行的内容了！*/
    // TODO: 下面这段代码，我认为写得极为漂亮！
    auto channel = shared_from_this();// 创建 当前类的智能指针shared_ptr对象，并被捕获到后续的lambda表达式中
    // 设置RPC超时定时器事件，因为是一次RPC调用，so不是可重复的任务
    m_timer_event = std::make_shared<TimerEvent>(my_rpc_controller->GetTimeout(), false, [my_rpc_controller, channel]() mutable -> void {
        if(my_rpc_controller->IsFinished()){
            channel.reset();
            return;
        }
        // rpc控制器 设置 取消本次RPC调用
        my_rpc_controller->StartCancel();
        // rpc控制器 设置 错误信息
        my_rpc_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout, time_out val[" + std::to_string(my_rpc_controller->GetTimeout())+ "]ms");
        // 执行客户端回调
        channel->CallBack();
        channel.reset();// 释放channel所占用的heap区资源
    });
    // 添加定时器任务到epoll上
    m_client->AddTimerEvent(m_timer_event);

    m_client->Connect([req_protocol, this]() mutable ->void {
        RpcController* my_rpc_controller = dynamic_cast<RpcController*>(GetController().get());
        if(GetTcpClient()->GetErrorCode() != 0){
            // 说明此时connect失败了
            // set rpc控制器的错误信息
            my_rpc_controller->SetError(GetTcpClient()->GetErrorCode(), GetTcpClient()->GetErrorInfo());
            // 打印错误日志
            ERRORLOG("MsgId[%s] | rpc client local_addr[%s] connect peer_addr[%s] error, error_code[%d], error_info[%s]", req_protocol->getMsgId().c_str(), 
                GetTcpClient()->GetLocalAddr()->toString().c_str(), GetTcpClient()->GetPeerAddr()->toString().c_str(), 
                GetTcpClient()->GetErrorCode(), GetTcpClient()->GetErrorInfo().c_str());
            CallBack();
            return;
        }
        INFOLOG("MsgId[%s] | rpc client local_addr[%s] connect peer_addr[%s] success", req_protocol->getMsgId().c_str(), 
            GetTcpClient()->GetLocalAddr()->toString().c_str(), GetTcpClient()->GetPeerAddr()->toString().c_str());
        // 发送消息，
        GetTcpClient()->Write(req_protocol, [req_protocol, this, my_rpc_controller](rocket::AbstractProtocol::s_ptr msg_ptr) mutable ->void {
            INFOLOG("MsgId[%s] | send rpc request success, call method name[%s], local_addr[%s], peer_addr[%s]",req_protocol->getMsgId().c_str(), 
                req_protocol->m_method_name.c_str(), GetTcpClient()->GetLocalAddr()->toString().c_str(), GetTcpClient()->GetPeerAddr()->toString().c_str());
            // 读回报，并打印xxx
            GetTcpClient()->Read(req_protocol->getMsgId(), [this, my_rpc_controller](AbstractProtocol::s_ptr msg_ptr) mutable ->void {
                std::shared_ptr<TinyPbProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPbProtocol>(msg_ptr);

                INFOLOG("MsgId[%s] | read rpc response success, call method name[%s], local_addr[%s], peer_addr[%s]", rsp_protocol->getMsgId().c_str(),
                    rsp_protocol->m_method_name.c_str(), GetTcpClient()->GetLocalAddr()->toString().c_str(), GetTcpClient()->GetPeerAddr()->toString().c_str());
                // 当成功收到RPC回包后，RPC调用成功，则第一时间取消RPC超时定时器任务事件，就算后面所设置的超时时间到来了也不会再执行该定时任务
                GetTimerEvent()->setCancle(true);

                if( !(GetResponse()->ParseFromString(rsp_protocol->m_pb_data)) ){
                    ERRORLOG("MsgId[%s] | re-serialize error", rsp_protocol->getMsgId().c_str());
                    // 只有controll设置了error，外层客户端才能知道是否这次RPC调用成功
                    my_rpc_controller->SetError(ERROR_FAILED_RE_SERIALIZE, "re-serialize error");
                    CallBack();
                    return;
                }
                if( rsp_protocol->m_error_code != 0 ){
                    ERRORLOG("MsgId[%s] |  failed to call rpc method[%s], error code[%d], error info[%s]",
                    rsp_protocol->getMsgId().c_str(), rsp_protocol->m_method_name.c_str(), 
                    rsp_protocol->m_error_code, rsp_protocol->m_error_info.c_str());
                    // 只有controll设置了error，外层客户端才能知道是否这次RPC调用成功
                    my_rpc_controller->SetError(rsp_protocol->m_error_code, rsp_protocol->m_error_info);
                    CallBack();
                    return;
                }
                
                INFOLOG("MsgId[%s] | rpc call success and done function will be run, local_addr[%s], peer_addr[%s]", rsp_protocol->getMsgId().c_str(), 
                    GetTcpClient()->GetLocalAddr()->toString().c_str(), GetTcpClient()->GetPeerAddr()->toString().c_str());
                // 此时，收到RPC回包，表示本次的RPC调用成功！(能走到这一步，表明 rsp_protocol->m_error_code == 0，也即调用成功的意思)

                // 当本次的RPC调用没有因为超时被取消 并且 RPC回调函数不为空时 ，执行RPC回调函数
                // if(!my_rpc_controller->IsCanceled() && GetClosure().get()){ // <==> if(rpc_controller_is_not_cancled && done != NULL){ done->Run(); }
                //     GetClosure()->Run();
                // } else { 
                //     ERRORLOG("done function is empty, so run it error");
                // }
                CallBack();
                // GetClosure().reset();// 析构 当前的 std::shared_ptr<RpcChannel> channel这个智能指针所管理(指向)的堆区内存。
            });// end m_client.Read()
        });// end m_client.Write()
    });// end m_client.Connect();// m_client连接成功之后，就会执行write 和 read
    
}

}// rocket