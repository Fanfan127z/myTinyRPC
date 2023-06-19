#include <iostream>
#include "../common/log.h"
#include "../common/config.h"
#include "../net/tcp/tcp_accepter.h"

int main(int argc, char* argv[]){

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");// 初始化创建全局 配置对象

    rocket::Logger::InitGlobalLogger();// 初始化创建全局 日志对象
    
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
    return 0;
}