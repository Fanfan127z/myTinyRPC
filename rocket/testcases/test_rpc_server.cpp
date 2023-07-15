#include <iostream>
#include <string>
#include <unistd.h>
#include <memory>
#include <google/protobuf/service.h>// use google::protobuf::RpcController
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
#include "../pb/order.pb.h"
using namespace std;

class OrderImpl : public Order {
public:
    void makeOrder(google::protobuf::RpcController* controller,
                    const ::makeOrderRequest* request,
                    ::makeOrderResponse* response,
                    ::google::protobuf::Closure* done){
        // 我们继承了 Order 基类之后，可以自定义去实现我们自己想实现的业务方法！
        // 我们想随便怎么写就怎么写
        INFOLOG("start service.method()");
        // test 超时rpc任务
        // INFOLOG("start sleep 3s");
        // sleep(3);
        // INFOLOG("end sleep 3s");
        if(request->price() < 10){
            response->set_ret_code(-1);
            response->set_res_info("Insufficient Balance");
            return;
        }
        response->set_order_id("20230712");
        INFOLOG(" end service.method()");
    }
};


void test_tcp_netaddr();
void test_tcp_server(uint16_t port);

int main(int argc, char* argv[]){

    rocket::Config::SetGlobalConfig("/home/ubuntu/projects/myTinyRPC2/myTinyRPC/rocket/conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();// 创建一个RPC服务 对象

    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);// 注册这个服务
    // rocket::RpcDispatcher::GetRpcDispatcher()->
    // test_tcp_server(std::stoi(std::string(argv[1])));
    uint16_t port = std::stoi(std::string(argv[1]));
    rocket::IPv4NetAddr::s_ptr addr = std::make_shared<rocket::IPv4NetAddr>("127.0.0.1", port); 
    rocket::TcpServer server(addr);
    INFOLOG("create addr[%s]", addr->toString().c_str());
    server.start();
    return 0;
}

void test_tcp_server(uint16_t port){
    rocket::IPv4NetAddr::s_ptr addr = std::make_shared<rocket::IPv4NetAddr>("127.0.0.1", port); 
    rocket::TcpServer server(addr);
    INFOLOG("create addr[%s]", addr->toString().c_str());
    server.start();
    
}

void test_tcp_netaddr(){
    rocket::IPv4NetAddr addr("127.0.0.1", 12345);
    printf("addr.toString() = %s\n", addr.toString().c_str());
    rocket::IPv4NetAddr addr2("127.0.0.1:12346");
    printf("addr2.toString() = %s\n", addr2.toString().c_str());
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;// IPV4
    serv_addr.sin_port = htons(10001);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rocket::IPv4NetAddr addr3(serv_addr);
    printf("addr3.toString() = %s\n", addr3.toString().c_str());
}