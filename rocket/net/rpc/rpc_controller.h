#ifndef ROCKET_NET_RPC_RPC_CONTROLLER_H
#define ROCKET_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h> // use google::protobuf::PpcController
#include <google/protobuf/stubs/callback.h> // use google::protobuf::Closure
#include <string>
#include <cstring>
#include "../tcp/net_addr.h"

namespace rocket{
/*
    RpcController类：
    主要作用：
    就是要做RPC调用过程中的一些配置，比如控制这次的RPC的超时时间，获取对端网络地址等信息 
    我们需要将RPC调用的过程中的一些关键配置信息参数写入RpcController类中，方便后续使用

    注意，我们这里只是去继承 google::protobuf::RpcController基类，然后重写对应的方法
*/
class RpcController : public google::protobuf::RpcController{
private:
    int32_t m_error_code {0};
    std::string m_error_info;
    std::string m_req_id;

    bool m_is_failed {false};// 判断这次RPC调用是否已经失败了
    bool m_is_cancled {false};// 判断这次RPC调用是否已经被取消

    NetAddrBase::s_ptr m_local_addr;// 该RPC调用的本地网络地址
    NetAddrBase::s_ptr m_peer_addr;// 该RPC调用的对端网络地址
    
    int m_time_out {1000};// 单位是ms，so 1000ms == 1s

    // TODO: if 后续有别的参数，可补充~
public:
    // RpcController() = default;
    // ~RpcController() = default;
public:
    void SetError(int32_t error_code, const std::string& error_info);
    inline int32_t GetErrorCode() const { return m_error_code; }
    inline std::string GetErrorInfo() const { return m_error_info; }
    inline void SetReqId(const std::string& req_id){ m_req_id = req_id; }
    inline std::string GetReqId() const { return m_req_id; }

    inline void SetLocalAddr(const NetAddrBase::s_ptr& addr) { m_local_addr = addr; }
    inline NetAddrBase::s_ptr GetLocalAddr() const { return m_local_addr; }
    inline void SetPeerAddr(const NetAddrBase::s_ptr& addr) { m_peer_addr = addr; }
    inline NetAddrBase::s_ptr GetPeerAddr() const { return m_peer_addr; }

    inline void SetTimeout(int timeout) { m_time_out = timeout; }
    inline int32_t GetTimeout() const { return m_time_out; }
    
public:
    // 以下是 继承自google标准库的RpcController虚基类的方法
    // Client-side methods ---------------------------------------------
    // These calls may be made from the client side only.  Their results
    // are undefined on the server side (may crash).
    // 以下是作为 RPC客户端 才能调用的方法：
    void Reset() override;// 复用之前的RpcController对象，以便于下一次的RPC调用
    bool Failed() const override;// 判断当前RPC调用是否成功
    std::string ErrorText() const override;
    void StartCancel() override;
    // Server-side methods ---------------------------------------------
    // These calls may be made from the server side only.  Their results
    // are undefined on the client side (may crash).
    // 以下是作为 RPC服务端 才能调用的方法：
    void SetFailed(const std::string& reason) override;
    bool IsCanceled() const override;
    void NotifyOnCancel(google::protobuf::Closure* callback) override;
};


}// rocket

// Client-side methods ---------------------------------------------
    // These calls may be made from the client side only.  Their results
    // are undefined on the server side (may crash).

    // Resets the RpcController to its initial state so that it may be reused in
    // a new call.  Must not be called while an RPC is in progress.
    // virtual void Reset();

    // After a call has finished, returns true if the call failed.  The possible
    // reasons for failure depend on the RPC implementation.  Failed() must not
    // be called before a call has finished.  If Failed() returns true, the
    // contents of the response message are undefined.
    // virtual bool Failed() const;

    // If Failed() is true, returns a human-readable description of the error.
    // virtual std::string ErrorText() const;

    // Advises the RPC system that the caller desires that the RPC call be
    // canceled.  The RPC system may cancel it immediately, may wait awhile and
    // then cancel it, or may not even cancel the call at all.  If the call is
    // canceled, the "done" callback will still be called and the RpcController
    // will indicate that the call failed at that time.
    // virtual void StartCancel();

    // Server-side methods ---------------------------------------------
    // These calls may be made from the server side only.  Their results
    // are undefined on the client side (may crash).

    // Causes Failed() to return true on the client side.  "reason" will be
    // incorporated into the message returned by ErrorText().  If you find
    // you need to return machine-readable information about failures, you
    // should incorporate it into your response protocol buffer and should
    // NOT call SetFailed().
    // virtual void SetFailed(const std::string& reason);

    // If true, indicates that the client canceled the RPC, so the server may
    // as well give up on replying to it.  The server should still call the
    // final "done" callback.
    // virtual bool IsCanceled() const;

    // Asks that the given callback be called when the RPC is canceled.  The
    // callback will always be called exactly once.  If the RPC completes without
    // being canceled, the callback will be called after completion.  If the RPC
    // has already been canceled when NotifyOnCancel() is called, the callback
    // will be called immediately.
    // NotifyOnCancel() must be called no more than once per request.
    // virtual void NotifyOnCancel(Closure* callback);

#endif // ROCKET_NET_RPC_RPC_CONTROLLER_H

