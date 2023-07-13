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
#include "order.pb.h"

using namespace std;
void test_conncect(uint16_t port);
void test_tcp_client2(uint16_t port);
int main(int argc, char* argv[]){

    rocket::Config::SetGlobalConfig("/home/ubuntu/projects/myTinyRPC/rocket/conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象
    
    // test_conncect(std::stoi(std::string(argv[1])));

    test_tcp_client2(std::stoi(std::string(argv[1])));
    return 0;
}
void test_tcp_client2(uint16_t port){
    rocket::IPv4NetAddr::s_ptr addr = std::make_shared<rocket::IPv4NetAddr>("127.0.0.1", port); 
    rocket::TcpClient client(addr);
    auto cb = [addr, &client]()->void{
        DEBUGLOG("test_tcp_client connect [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::TinyPbProtocol> msg = std::make_shared<rocket::TinyPbProtocol>();
        msg->setRequestId("99998888");
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
            DEBUGLOG("read msg success, req_id[%s], msg[%s]", msg->getRequestId().c_str(), msg->m_pb_data.c_str());
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