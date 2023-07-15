#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include "../common/log.h"
#include "../common/config.h"
#include "../net/tcp/tcp_connection.h"
#include "../net/tcp/tcp_accepter.h"
#include "../net/tcp/tcp_server.h"
#include "../net/tcp/tcp_client.h"
#include "../net/tcp/net_addr.h"
#include "../net/codec/abstract_codec.h"
#include "../net/codec/abstract_protocol.h"
#include "../net/string_codec.h"
#include "../net/codec/tiny_pb_codec.h"
#include "../net/codec/tiny_pb_protocol.h"
#include "../net/rpc/rpc_dispatcher.h"
#include "../net/rpc/rpc_channel.h"
#include "../net/rpc/rpc_closure.h"
#include "../pb/order.pb.h"

using namespace std;
void test_conncect(uint16_t port);
void test_tcp_client2(uint16_t port);
void test_rpc_channel(uint16_t port){
    // rocket::IPv4NetAddr::s_ptr addr = std::make_shared<rocket::IPv4NetAddr>("127.0.0.1", port); 
    // TODO: 创建 RpcChannel，屏蔽掉底层客户端发起RPC请求并收回包的代码细节，并封装好 隔层回调函数，并注意到了回调好函数中裸指针可能会被释放的缺点，
    // 都改用shared_ptr引用计数来do回调函数的传参，这样就会防止出现回调隔层回调函数时发送core dump了。这个技术细节和感悟 可以写入简历
    NEWRPCCHANNEL(std::string("127.0.0.1:" + std::to_string(port)), channel);
    // rocket::RpcChannel::s_ptr channel = std::make_shared<rocket::RpcChannel>(addr);

    // 构建 RPC控制器
    // std::shared_ptr<rocket::RpcController> controller = std::make_shared<rocket::RpcController>();
    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("99998888");// 设置请求的消息号
    controller->SetTimeout(1000); // TODO:设置10s的超时时间，如果这次RPC调用超过这个时间还没有调用完成 并 返回结果的话，就强制结束，标志RPC调用失败
    
    // 构建 RPC请求
    // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    // 封装为宏定义，这样子就不用暴露我使用到shared_ptr来管理request和response的细节，防止调用方没有使用智能指针导致内存的泄露问题！
    NEWMESSAGE(makeOrderRequest, request);
    request->set_price(100);
    request->set_goods("apple");
    // 构建 RPC响应（一开始是空的，因为 需要等待RPC回包设置信息 给到响应体）
    // std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();
    NEWMESSAGE(makeOrderResponse, response);
    // 构建 RPC回调函数类，执行回调函数done
    std::shared_ptr<rocket::RpcClosure> closure = std::make_shared<rocket::RpcClosure>([&controller, request, &response, &channel]() mutable -> void{
        if(controller->GetErrorCode() == 0){
            INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 调用RPC成功，然后就可以执行 你自定义的 业务逻辑了
            // if(response->order_id() == "xxx"){
            //     // do something
            // }
        } else {
            // RPC调用失败的话response会是空，不信你可以do如下打印
            ERRORLOG("call rpc failed, request[%s], response[%s], error code[%d], error info[%s]", request->ShortDebugString().c_str()
                , response->ShortDebugString().c_str(), controller->GetErrorCode(), controller->GetErrorInfo().c_str());
            // <==>
            // ERRORLOG("call rpc failed, request[%s], response[NULL], error code[%d], error info[%s]", request->ShortDebugString().c_str(),
            //     controller->GetErrorCode(), controller->GetErrorInfo().c_str());
        }
        INFOLOG("now rpc-client exit eventLoop");// 执行完一次RPC调用之后，无论是否成功，都退出eventloop，结束调用，释放资源
        channel->GetTcpClient()->Stop();
        channel.reset(); // 用&channel 捕获 或者 用mutable休息该lambda都可以修改！
    });

    // channel->Init(controller, request, response, closure);
    // 创建Order_Stub对象（然后才能调用RPC方法）
    // Order_Stub stub(channel.get());
    // 调用RPC method
    // stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
    // TODO: 实际上，stub.makeOrder()内部会调用到channel->CallMethod(controller, request, response, closure)方法
    // 必须在使用宏定义 NEWRPCCHANNEL 创建了channel之后再使用宏定义 CALLRPC 调用RPC服务
    CALLRPC(makeOrder, controller, request, response, closure);
}
int main(int argc, char* argv[]){

    rocket::Config::SetGlobalConfig("/home/ubuntu/projects/myTinyRPC2/myTinyRPC/rocket/conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象
    
    // test_conncect(std::stoi(std::string(argv[1])));

    // test_tcp_client2(std::stoi(std::string(argv[1])));
    test_rpc_channel(std::stoi(std::string(argv[1])));
    return 0;
}
void test_tcp_client2(uint16_t port){
    rocket::IPv4NetAddr::s_ptr addr = std::make_shared<rocket::IPv4NetAddr>("127.0.0.1", port); 
    rocket::TcpClient client(addr);
    auto cb = [addr, &client]()->void{
        DEBUGLOG("test_tcp_client connect [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::TinyPbProtocol> msg = std::make_shared<rocket::TinyPbProtocol>();
        msg->setMsgId("99998888");
        msg->m_pb_data = "test pb data"; 

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        if(!request.SerializeToString(&(msg->m_pb_data))){
            ERRORLOG("serialize request msg object to (pb_data)string error");
            return;
        }
        msg->m_method_name = "Order.makeOrder";

        // 发送消息，
        client.Write(msg, [request](rocket::AbstractProtocol::s_ptr msg_ptr)->void{
            DEBUGLOG("send msg success, request[%s]", request.ShortDebugString().c_str());
        });
        
        // 读回报，并打印xxx
        client.Read(std::string("99998888"), [](rocket::AbstractProtocol::s_ptr msg_ptr)->void{
            std::shared_ptr<rocket::TinyPbProtocol> msg = std::dynamic_pointer_cast<rocket::TinyPbProtocol>(msg_ptr);// 向下类型转换
            DEBUGLOG("read msg success, req_id[%s], msg[[%s]", msg->getMsgId().c_str(), msg->m_pb_data.c_str());
        });
        makeOrderResponse response;
        
        if(!response.ParseFromString(msg->m_pb_data)){
            ERRORLOG("re-serialize response msg object from (pb_data)string error");
            return;
        }
        DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
    };
    client.Connect(cb);
}


void test_conncect(uint16_t port){

    // 1.调用 connect 连接 RPC-server
    // 2.write 一个字符串
    // 3.等待 read 返回结果
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        // perror("socket");
        ERRORLOG("client socket error");
        exit(0);
    }
    // 2.连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));// 初始化一下，防止数据有问题！
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);// 大端网络字节序
    // 将字符串类型的IP地址转换为大端
    string IP = "127.0.0.1";// "101.33.219.185" 
    inet_pton(AF_INET, IP.c_str(), &server_addr.sin_addr.s_addr);

    // 连接
    int ret = connect(cfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == -1)
    {
        // perror("connect");
        ERRORLOG("client connect error");
        exit(0);
    }
    // 3.和服务端通信
    int num = 0;
    string msg = "";
    while(1){
        // 发送数据
        char buf[1024];
        sprintf(buf,"hello, rocket [%d]", num++);
        int rt = write(cfd, buf, strlen(buf)+1);// +上'\0'字符串的结束符
        INFOLOG("success write %d bytes, %s", rt, buf);
        // 接收数据
        memset(buf, 0, sizeof(buf));
        int len = read(cfd, buf, sizeof(buf));
        if(len > 0){
            INFOLOG("success read %d bytes, [%s]", rt, buf);
        }else if(len == 0){
            INFOLOG("server断开连接了...");
            break;
        }else{
            // len < 0，此时接收数据失败
            // perror("read");
            INFOLOG("client read error");
            break;
        }
        sleep(1);// 每间隔1s钟发送一次数据
    }
    close(cfd); 
}