#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/time.h> // 获取时间
#include <time.h>
#include <sstream>    // 使用ssream对象,用于进行字符串流的输入输出操作
#include "log.h"
#include "util.h"






namespace rocket{

    static Logger* g_logger = nullptr;

    std::string LogLevel2String(LogLevel logLevel){
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
           << "[" << std::string(__FILE__) << ":" <<  __LINE__ << "]\t";
        
        return ss.str();
    }

    LogEvent::LogEvent(const LogLevel& logLevel){
        this->m_log_level = logLevel;
    }
    /*
        Logger class function definition
    */

    void Logger::pushLog(const std::string& msg){
        m_buffers.push(msg);
    }
    Logger* Logger::GetGlobalLogger(){
        if(g_logger != nullptr)return g_logger;// 不是空，直接返回
        // 是空的话，那就new 一个Logger日志器
        g_logger = new Logger();
        return g_logger;
    }
    void Logger::log(){
        while(!this->m_buffers.empty()){
            std::string msg = m_buffers.front();
            m_buffers.pop();
            printf("%s\n",msg.c_str());
        }
        
    }
}
