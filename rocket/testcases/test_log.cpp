#include <iostream>
#include <string>
#include "../common/log.h"
#include "../common/util.h"
int main(void){

    // DEBUGLOG(,"11");

    // 我现在主要就是没法调试我自己的项目！！！
    // std::string str = "test log %s\n";用这个穿进去函数模板里面，是没法成功do编译的！
    std::string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString();
    msg += rocket::formatString("test log %s\n", "11"); 
    rocket::Logger::GetGlobalLogger()->pushLog(msg); 
    rocket::Logger::GetGlobalLogger()->log();
    
    return 0;
}