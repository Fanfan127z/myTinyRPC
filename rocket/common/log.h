#ifndef ROCKET_LOG_H
#define ROCKET_LOG_H

#include <unistd.h>
#include <cstring>
#include <string>
#include <queue>
#include <pthread.h>

namespace rocket{
    // 注：__VA_ARGS__是C语言中的可变参数宏，它允许宏接受可变数量的参数。

    #define DEBUGLOG(str, ...)\
        std::string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + rocket::formatString(str, ##_VA_ARGS__); \
        rocket::Logger::GetGlobalLogger()->pushLog(msg); \
        rocket::Logger::GetGlobalLogger()->log(); 
    /* 
        日志字符串格式转换中间函数:
            用于格式化字符串。它使用了可变参数模板，可以接受任意数量和类型的参数。函数首先使用snprintf函数计算格式化后的字符串长度，然后创建一个字符串对象，并调整其大小以适应格式化后的字符串。最后，
            使用snprintf函数将格式化后的字符串写入到字符串对象中，并返回该对象。
    */ 
    template<class... Args>
    std::string formatString(const char* str, Args&& ... args){ // 打印日志时，其实就是调用这个formatString方法就ok了！
        int size = snprintf(nullptr,0,str,args ...);
        std::string result;
        if(size > 0){
            result.resize(size);
            snprintf(&result[0], size+1, str , args...);
        }
        return result;
    }

    

    /*
        日志级别
    */
    enum LogLevel{
        Debug = 1,
        Info  = 2,
        Error = 3
    };
    /*
        日志到字符串的转换
    */
    std::string LogLevel2String(LogLevel logLevel);
    /*
        日志事件
            LogEvent:
            ---
            文件名、行号
            MsgNo(每一次的RPC请求信息)
            Process id 
            Thread id 
            time(日期时间，准确到毫秒ms)
            自定义消息(log中需要打印到额外的信息)
            ---
    */
    class LogEvent{
    private:
        std::string m_fileName; // 文件名
        int32_t m_file_line;    // 行号
        pid_t   m_pid;          // 进程号
        pthread_t m_thread_id;  // 线程号
        LogLevel m_log_level;   // 日志级别
    public:
        // LogEvent() = default;
        LogEvent(const LogLevel& logLevel);
        inline std::string getFileName()const { return m_fileName;}
        inline LogLevel getLogLevel()const { return m_log_level; }
        std::string toString();
        
        ~LogEvent() = default;
    };
    /*
        Logger 日志器
            1-提供打印日志的方法
            2-设置日志的输出路径
    */
    class Logger{
    private:
        std::string m_log_path;
        LogLevel m_set_log_level;
        std::queue<std::string> m_buffers;
    public:
        Logger() = default;
        inline std::string getLogPath()const { return m_log_path; }
        
        void pushLog(const std::string& msg);

        static Logger* GetGlobalLogger();
        void log();
        ~Logger() = default;
    };
}

#endif // ROCKET_LOG_H