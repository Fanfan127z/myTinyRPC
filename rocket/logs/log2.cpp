#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/time.h> // 获取时间
#include <time.h>
#include <sstream>    // 使用sstream对象,用于进行字符串流的输入输出操作
#include <functional>
#include <assert.h>
#include "log.h"
#include "util.h"
#include "../common/config.h"
#include "../net/eventloop.h"

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
    Logger class function definition:
*/
Logger::Logger(const LogLevel& logLevel) : m_set_log_level(logLevel){
    
}

// Logger* Logger::GetGlobalLogger(){// static
std::shared_ptr<Logger> Logger::GetGlobalLogger(){// static
    return g_logger;
}
void Logger::InitGlobalLogger(){// static
    
    // if(g_logger != nullptr)return g_logger;// 不是空，直接返回
    // 是空的话，那就new 一个Logger日志器
    
    LogLevel global_log_level = String2LogLevel(Config::GetGlobalConfig()->GetLogLevel());
    printf("Init log level [%s]\n", LogLevel2String(global_log_level).c_str());
    // printf("global_log_level is  [%d]\n", global_log_level);// test codes!
    
    // g_logger = new Logger(global_log_level);
    g_logger = std::make_shared<Logger>(global_log_level);
    g_logger->init();
}
// TODO: 这里又来一个init的原因就是：日志Logger对象(泛化来说，就是某个对象)的构造 的循环依赖！
// 二次初始化 logger，至于why这么做，原因就在于我们在Logger中开启的定时去同步日志到 异步日志总的buffers中 的 任务
// 这个任务里面又打印了log，如果void Logger::init()这个函数里面的事情你放在Logger(const LogLevel& logLevel)中
// 此时Logger对象还没有创建完毕，但是此时你timerEvent里面又调用了DEBUGLOG or INFOLOG等打印日志的操作，此时就会产生 TODO: 循环依赖问题
// 这就是为什么，很多时候，我们把 构造函数 的一些操作的代码，会被 分开 为1个or多个 对应的成员函数 去do！
void Logger::init(){
    // new 一个 异步日志对象
    std::string file_name = Config::GetGlobalConfig()->GetLogFileName();
    std::string file_path = Config::GetGlobalConfig()->GetLogFilePath();
    size_t max_size = Config::GetGlobalConfig()->GetLogFileMaxSize();
    m_async_logger = std::make_shared<AsyncLogger>(file_name, file_path, max_size);
    /* TimerEvent(int64_t interval, bool is_repeated, const std::function<void()>& task); */
    // 每间隔 (log_sync_interval) ms，重复 把同步日志push进去异步日志队列 的事情
    size_t log_sync_interval = Config::GetGlobalConfig()->GetLogSyncInterVal();
    // std::bind(&Logger::SyncLog, this)这种写法的含义就是，因为SyncLog是属于Logger的成员函数，且不是static静态方法
    // so需要用bind把是属于哪个对象的范畴动态地绑定进去！这样子才知道是调用属于哪个Logger对象的成员函数
    // ==> this->SyncLog, if 是static函数的话，就直接传 Logger::SyncLog进去就ok了，因为static成员函数是属于all对象的！不受任何对象的制约
    m_timer_event = std::make_shared<TimerEvent>(log_sync_interval, true, std::bind(&Logger::SyncLog, this));
    EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);// 添加 定时(周期) push日志到异步总日志队列中 的任务
}
// 同步 Logger的m_buffers日志 到异步日志队列的总buffers(m_all_log_buffers)的队尾
void Logger::SyncLog(){
    std::vector<std::string> tmp_buffers;
    ScopeMutex<Mutex> lock(this->m_mutex);
    tmp_buffers.swap(this->m_buffers);// 此时，this->m_buffers变为空，tmp_buffers拿到之前this->m_buffers存放的日志strings
    lock.unlock();
    if(!tmp_buffers.empty()){// 这里严谨一点，非空，才push日志同步 给 异步日志总buffers中去打印
        printf("tmp_buffers.size()=%ld\n", tmp_buffers.size());
        this->m_async_logger->pushLogBuffer(tmp_buffers);
    }
}

void Logger::pushLog(const std::string& msg){
    ScopeMutex<Mutex> lock(this->m_mutex);
    m_buffers.push_back(msg);
    // lock.unlock();// 其实你自己不手动解锁也是一样的，因为这里退出函数之后肯定会 析构自动调用unlock的！
}
void Logger::log(){
    ScopeMutex<Mutex> lock(this->m_mutex);
    lock.lock();
    // 为什么不直接用 m_mutex.lock 和 unlock呢？因为这次复合RAII
    // 我试过了，直接用m_mutex也是ok的哈！

    // std::queue<std::string> tmp_buffers = this->m_buffers;// 这样子搞会出现因为共享buffer在子线程和主线程中因为
    // 所用栈区都是独立的，因此看到的内容是不一样的！从而会导致出某些log重复打印的bugs！so 不要这么干！
    // 不要迷信权威，不要只是迷信大佬的代码！一定要思考，为什么大佬要这么干！！！
    // while(!m_buffers.empty()){
    //     std::string msg = m_buffers.front();
    //     m_buffers.pop();
    //     // printf("void Logger::log():%s\n",msg.c_str()); // 这只是一个test codes，后面肯定不是在终端打印这个，而是在日志文件中打印这个！
    //     printf("%s",msg.c_str());// 现在是测试阶段，所以只是简单的打印到终端
    //     // 后面肯定是要实现打印到具体的.log日志文件并滚动打印的！（也即当日志太大了就需要换一个日志文件继续打印了！）
    //     // 而且，后续我也会改进这个log日志模块，把他改成异步的，启动别的子线程去专门定时打印日志的这样的功能！
    // }
    for(auto& msg : m_buffers){
        printf("%s",msg.c_str());
    }
    m_buffers.clear();
    lock.unlock();
}

/*
    AsyncLogger class function definition:
*/
AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, size_t max_size)
            :m_log_file_name(file_name), m_log_file_path(file_path), m_max_log_file_size(max_size){
    // 初始化信号量
    sem_init(&m_sem, 0, 0);
    
    // 创建一个属于该异步日志的线程
    assert(pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this) == 0);

    // 初始化条件变量
    // assert(pthread_cond_init(&m_cond, nullptr) == 0);

    // 等待新线程创建成功去执行Loop
    sem_wait(&m_sem);
}
// 把日志 同步添加到 异步的 总日志队列 中
void AsyncLogger::pushLogBuffer(const std::vector<std::string>& buffers){
    ScopeMutex<Mutex> lock(m_mutex);
    m_all_log_buffers.push(buffers);
    lock.unlock();
    printf("sync log to async final_log_buffers\n");
    // 注：此时有日志数据（生产了日志数据），需要唤醒 异步日志线程，去打印异步日志（消费日志数据）
    // 简单的 生产者-消费者 模型 
    pthread_cond_signal(&m_cond);// 这里本就只有一个cond条件变量，so不需要使用 pthread_cond_broadcast
    printf("pthread_cond_signal\n");
}


// 异步日志就会无限循环执行这个loop()函数，这是异步日志类AsyncLogger的核心func
void* AsyncLogger::Loop(void* arg){
    /* logic:
        将AsyncLogger::m_buffer中的全部数据打印到文件，然后执行这个loop的线程休眠，直到有新的数据到来，再重复这个过程
    */
    // 从传入参数arg拿到AsyncLogger指针
    AsyncLogger* logger = (AsyncLogger*)(arg);
    // 初始化条件变量
    assert(pthread_cond_init(&(logger->m_cond), nullptr) == 0);
    // 唤醒 AsyncLogger构造函数
    sem_post(&logger->m_sem);

    while(true){
        ScopeMutex<Mutex> lock(logger->m_mutex);
        while(logger->m_all_log_buffers.empty()){
            // 只要当前异步日志buffer中的数据为空，就一定等待唤醒，只要没有唤醒，就一直卡死在这
            pthread_cond_wait(&(logger->m_cond), logger->m_mutex.getRawMutex());
            // 一旦有 pthread_cond_signal(&m_cond) or pthread_cond_broadcast(&m_cond) 唤醒当前的条件变量
            // 此时 logger->m_all_log_buffers就不为空了，此时就break出 内while循环，出去打印日志去了
        }
        printf("pthread_cond_wait back\n");
        // 一旦跳出上面的内while循环，就说明此时有数据可写进日志文件了，也即要打印出去文件中了（要消费了）

        // 取最前面的一个日志数据
        std::vector<std::string> tmp_one_log_buffers;
        tmp_one_log_buffers.swap(logger->m_all_log_buffers.front());
        logger->m_all_log_buffers.pop();

        lock.unlock();// 及时地释放锁头，提高程序性能
        // 先获取当前时间
        // int64_t now = getNowMs();
        struct timeval now;
        memset(&now, 0, sizeof(now));// 先初始化now，防止异常
        gettimeofday(&now, NULL);

        struct tm now_time;
        memset(&now_time, 0, sizeof(now_time));// 先初始化now_time，防止异常
        localtime_r(&(now.tv_sec), &now_time);
        // 将日期格式化
        const char* date_format = "%Y%m%d";
        char date[32];
        memset(date, 0, sizeof(date));// 先初始化date，防止异常
        strftime(date, sizeof(date), date_format, &now_time);

        if(std::string(date) != logger->m_date){
            logger->m_logfile_num = 0;// 序号重新开始计算
            logger->m_re_open_flag = true;// 需要重复打开
            logger->m_date = std::string(date);
        }
        if(logger->m_file_handler == nullptr){
            logger->m_re_open_flag = true;// 需要重复打开，也可能是 之前还没有打开某个文件 的case
        }
        std::stringstream ss;
        ss << logger->m_log_file_path << "/" << logger->m_log_file_name << "_"
           << std::string(date) << "__";//  << ".log"
        std::string final_log_file_name = ss.str() + std::to_string(logger->m_logfile_num) + ".log";

        if(logger->m_re_open_flag){// m_re_open_flag为true就代表需要重新打开一个文件
            // 如果已经打开了，就先关闭
            if(logger->m_file_handler != nullptr){
                fclose(logger->m_file_handler);
            }
            // 打开文件(以追加“a”的方式打开，因为有可能这个文件名的log文件原来已经存在)
            // linux-terminal：man fopen可查看详细 fopen() 系统调用的细节内容
            logger->m_file_handler = fopen(final_log_file_name.c_str(), "a");
            logger->m_re_open_flag = false;// set为false
        }
        // 判断当前打开的文件大小是否超过我们的设定
        if((size_t)( ftell(logger->m_file_handler) ) >= logger->m_max_log_file_size){
            // 此时就需要切换并打开新的日志文件了
            fclose(logger->m_file_handler);
            // 此时，日志文件序列号+1
            logger->m_logfile_num++;
            // 重新设置日志文件名
            final_log_file_name = ss.str() + std::to_string(logger->m_logfile_num) + ".log";
            // 并 重新打开文件
            logger->m_file_handler = fopen(final_log_file_name.c_str(), "a");
            logger->m_re_open_flag = false;// set为false
        }
        // 打印日志
        for(auto& tmp_one_log : tmp_one_log_buffers){
            if(!tmp_one_log.empty()){
                fwrite(tmp_one_log.c_str(), 1, tmp_one_log.length(), logger->m_file_handler);
            }
        }
        /* do数据的同步：
            数据同步：fflush函数可以用于确保输出设备和文件的同步。当数据通过网络传输或者读写文件时，
            数据可能会在缓冲区中长时间滞留，而不被立即写入设备或文件。使用fflush函数可以确保数据被立即写入设备或文件，
            从而保持设备和文件的同步。*/
        // 立即将data刷新到磁盘
        // logger->Flush();
        assert(fflush(logger->m_file_handler) == 0);// 刷盘操作必须成功，否则就报错
        if(logger->m_stop_flag) break;// 暂停并退出异步日志的打印

        printf("AsyncLogger while looping\n");
    }// end while

    return nullptr;// 作为线程执行的函数，必须是void*(void*)类型
}
// 暂停并退出异步日志的打印
void AsyncLogger::Stop(){
    m_stop_flag = true;
}

// 持久化写入到日志文件数据到磁盘
void AsyncLogger::Flush(){
    if(m_file_handler != nullptr){// 打开的文件句柄存在才能拿来do刷盘持久化的操作
        assert(fflush(m_file_handler) == 0);// 刷盘操作必须成功，否则就报错
    }
}

AsyncLogger::~AsyncLogger(){
    // INFOLOG("~AsyncLogger()");
    sem_destroy(&m_sem);
    pthread_cond_destroy(&m_cond);
}

}// rocket
