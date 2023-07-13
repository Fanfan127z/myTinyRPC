#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <sys/time.h> // 获取时间
#include <time.h>
#include <sstream>    // 使用sstream对象,用于进行字符串流的输入输出操作

#include "log.h"
#include "util.h"
#include "../common/config.h"

namespace rocket{

    // static Logger* g_logger = nullptr;// 这样会lead to memory leakage
    static std::shared_ptr<Logger> g_logger = nullptr;

    std::string LogLevel2String(const LogLevel& logLevel){
        std::string ret;
        switch (logLevel){
        case Debug:
            ret = "DEBUG";break;
        case Info:
            ret = "INFO";break;
        case Error:
            ret = "ERROR";break;        
        default: 
            ret = "UNKNOWN";break;
        }
        return ret;
    }
    LogLevel String2LogLevel(const std::string& str){
        LogLevel ret;
        if(str == "Debug")ret = LogLevel::Debug;
        else if(strcmp(str.c_str(),"Info") == 0)ret = LogLevel::Info;
        else if(strcmp(str.c_str(),"Error") == 0)ret = LogLevel::Error;
        else ret = LogLevel::Unknown;
        return ret;
    }
    
    /*
        LogEvent class function definition
    */
    std::string LogEvent::toString(){
        struct timeval tv_now_time;
        gettimeofday(&tv_now_time,nullptr);
        struct tm  tm_info;
        // localtime()
        localtime_r(&tv_now_time.tv_sec,&tm_info);
        // strftime函数是用于将时间格式化为字符串的C语言函数
        char buf[128];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
        // printf("当前时间为：%s\n", buffer);// test codes
        // test codes :
        // printf("Current date and time: %s", asctime(&tm_info));
        // printf("Current year: %d\n", (&tm_info)->tm_year + 1900);
        // printf("Current month: %d\n", (&tm_info)->tm_mon + 1);
        // printf("Current day: %d\n", (&tm_info)->tm_mday);
        // printf("Current hour: %d\n", (&tm_info)->tm_hour);
        // printf("Current minute: %d\n", (&tm_info)->tm_min);
        // printf("Current second: %d\n", (&tm_info)->tm_sec);
        // printf("Current millisecond: %ld\n", tv_now_time.tv_usec / 1000);
        int ms = tv_now_time.tv_usec / 1000;
        std::string time_str(buf);
        time_str += "." + std::to_string(ms);// 构造准确的时间字符串
        this->m_pid = getpid();
        this->m_thread_id = getThreadId();

        std::stringstream ss;
        ss << "[" << LogLevel2String(this->m_log_level) << "]\t"
           << "[" << time_str << "]\t"
           << "[" << this->m_pid << ":" << this->m_thread_id << "]\t";
        //    << "[" << std::string(__FILE__) << ":" <<  __LINE__ << "]\t";
        
        return ss.str();
    }

    LogEvent::LogEvent(const LogLevel& logLevel){
        this->m_log_level = logLevel;
    }
    /*
        Logger class function definition
    */

    void Logger::pushLog(const std::string& msg){
        ScopeMutex<Mutex> lock(this->m_mutex);
        m_buffers.push(msg);
        // lock.unlock();// 其实你自己不手动解锁也是一样的，因为这里退出函数之后肯定会 析构自动调用unlock的！
    }
    // Logger* Logger::GetGlobalLogger(){// static
    std::shared_ptr<Logger> Logger::GetGlobalLogger(){// static
        return g_logger;
    }
    void Logger::InitGlobalLogger(){// static
        
        // if(g_logger != nullptr)return g_logger;// 不是空，直接返回
        // 是空的话，那就new 一个Logger日志器
        
        LogLevel global_log_level = String2LogLevel(Config::GetGlobalConfig()->getLogLevel());
        printf("Init log level [%s]\n", LogLevel2String(global_log_level).c_str());
        // printf("global_log_level is  [%d]\n", global_log_level);// test codes!
        
        // g_logger = new Logger(global_log_level);
        g_logger = std::make_shared<Logger>(global_log_level);

    }
    void Logger::log(){
        ScopeMutex<Mutex> lock(this->m_mutex);
        lock.lock();
        // 为什么不直接用 m_mutex.lock 和 unlock呢？因为这次复合RAII
        // 我试过了，直接用m_mutex也是ok的哈！

        // std::queue<std::string> tmp_buffers = this->m_buffers;// 这样子搞会出现因为共享buffer在子线程和主线程中因为
        // 所用栈区都是独立的，因此看到的内容是不一样的！从而会导致出某些log重复打印的bugs！so 不要这么干！
        // 不要迷信权威，不要只是迷信大佬的代码！一定要思考，为什么大佬要这么干！！！
        while(!m_buffers.empty()){
            std::string msg = m_buffers.front();
            m_buffers.pop();
            // printf("void Logger::log():%s\n",msg.c_str()); // 这只是一个test codes，后面肯定不是在终端打印这个，而是在日志文件中打印这个！
            printf("%s",msg.c_str());// 现在是测试阶段，所以只是简单的打印到终端
            // 后面肯定是要实现打印到具体的.log日志文件并滚动打印的！（也即当日志太大了就需要换一个日志文件继续打印了！）
            // 而且，后续我也会改进这个log日志模块，把他改成异步的，启动别的子线程去专门定时打印日志的这样的功能！
        }
        // lock.unlock();
    }
}// rocket
