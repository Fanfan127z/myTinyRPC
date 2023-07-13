#ifndef ROCKET_RPC_RPC_CLOSURE_H
#define ROCKET_RPC_RPC_CLOSURE_H
#include <functional>
#include <google/protobuf/stubs/callback.h>// use google::protobuf::Closure
namespace rocket{
/*
    RpcClosure类的主要作用是：执行RPC调用的回调函数！
    继承自google::protobuf::Closure类，并重写方法run()
*/
class RpcClosure : public google::protobuf::Closure {
private:
    std::function<void()> m_callback {nullptr};// 回调函数
public:
    // RpcClosure() = default;
    // ~RpcClosure() = default;
    
    void Run() override ;
};

}// rocket
#endif // ROCKET_RPC_RPC_CLOSURE_H 