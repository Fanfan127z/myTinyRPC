#include <iostream>
#include <string>
#include "../common/log.h"
#include "../common/config.h"
#include "../net/tcp/tcp_accepter.h"
#include "../net/tcp/tcp_server.h"
#include "../net/tcp/tcp_connection.h"

void test_tcp_netaddr();
void test_tcp_server(uint16_t port);

int main(int argc, char* argv[]){

    rocket::Config::SetGlobalConfig("/home/ubuntu/projects/myTinyRPC/rocket/conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象
    
    test_tcp_server(std::stoi(std::string(argv[1])));
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