#include "rpc_controller.h"
namespace rocket{


void RpcController::SetError(int32_t error_code, const std::string& error_info){
    m_error_code = error_code;
    m_error_info = error_info;
}

// Client-side methods ---------------------------------------------
// These calls may be made from the client side only.  Their results
// are undefined on the server side (may crash).
// 以下是作为 RPC客户端 才能调用的方法：

// 复用之前的RpcController对象，以便于下一次的RPC调用

void RpcController::Reset(){
    m_error_code = 0;
    m_error_info = "";
    m_req_id = "";
    m_is_failed = false;// 判断这次RPC调用是否已经失败了
    m_is_cancled = false;// 判断这次RPC调用是否已经被取消
    m_local_addr = nullptr;// 该RPC调用的本地网络地址
    m_peer_addr = nullptr;// 该RPC调用的对端网络地址
    m_time_out = 1000;// 单位是ms，so 1000ms == 1s
}

// 判断当前RPC调用是否成功
bool RpcController::Failed() const {
    return m_is_failed;
}

// 判断当前RPC调用是否成功
std::string RpcController::ErrorText() const {
    return m_error_info;
}

void RpcController::StartCancel(){
    m_is_cancled = true;
}

// Server-side methods ---------------------------------------------
// These calls may be made from the server side only.  Their results
// are undefined on the client side (may crash).
// 以下是作为 RPC服务端 才能调用的方法：
void RpcController::SetFailed(const std::string& reason){
    m_error_info = reason;
}

bool RpcController::IsCanceled() const {
    return m_is_cancled;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback){

}

}// rocket