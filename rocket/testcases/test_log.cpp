#include <iostream>
#include <string>
#include <pthread.h>

#include "../common/log.h"
#include "../common/util.h"
#include "../common/config.h"

// #include "a.h"
void* func(void* arg){

    DEBUGLOG("debug this is thread in %s","func");
    INFOLOG("info this is thread in %s","func");
    // rocket::Logger::GetGlobalLogger()->log();
    return nullptr;
}
int main(void){
    // A * pa = A::func_get_global_A();

 
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    pthread_t th1;
    pthread_create(&th1,nullptr,func,nullptr);
    pthread_join(th1,nullptr);
    // 终于调试到没有bug了我的log！
    // rocket::Logger::GetGlobalLogger()->log();
    DEBUGLOG("test debug log %s","11");
    INFOLOG("test info log %s","12");
    rocket::Logger::GetGlobalLogger()->log();
    
    // int ii=0;
    // while(ii < 6){
    //     pthread_t th1;
    //     pthread_create(&th1,nullptr,func,nullptr);
    //     pthread_join(th1,nullptr);
    //     // 终于调试到没有bug了我的log！
    //     // rocket::Logger::GetGlobalLogger()->log();
    //     DEBUGLOG("test debug log %s","11");
    //     INFOLOG("test info log %s","12");
    //     rocket::Logger::GetGlobalLogger()->log();
    //     // ++ii;
    // }
    
    // 注意了，这里的log只能打印一次，否则你中途去log的话，很容易会导致抢着log导致打印混乱的问题！

    // 对于这个全局的指针对象，我应该用shared_ptr来管理！这样才能够防止内存泄漏的问题！

    // 我现在主要就是没法调试我自己的项目！！！
    // std::string str = "test log %s\n";用这个穿进去函数模板里面，是没法成功do编译的！
    // std::string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString();
    // msg += rocket::formatString("test log %s\n", "11"); 
    // rocket::Logger::GetGlobalLogger()->pushLog(msg); 
    // rocket::Logger::GetGlobalLogger()->log();

    // delete pa;
    return 0;
}