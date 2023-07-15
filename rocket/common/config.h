#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <string> // 存放 配置变量
namespace rocket{

class Config{
private:
    // 配置变量 们
    std::string m_log_level;// 日志类型
    
    std::string m_log_file_path;// 日志文件路径
    std::string m_log_file_name;// 日志文件名
    size_t m_log_file_max_size {0};// 日志文件最大能存储的总的数据的字节数，单位：bytes，1 bytes == 8 bits , set 一个log最大为 10000000 bytes ≈ 9.5MB(兆字节)就基本够测试用，但实际生产环境可能1G或2G的
    size_t m_log_sync_interval {0};// 日志同步间隔，单位：ms，这个值不能太大，因为太大了可能出现日志不同步，然后丢失的case，这里一般set为几百ms即可

public:
    Config() = default;
    Config(const char * xmlFilePath);
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char * xmlFile);

    inline std::string GetLogLevel() const { return m_log_level; }
    inline std::string GetLogFilePath() const { return m_log_file_path; }
    inline std::string GetLogFileName() const { return m_log_file_name; }
    inline size_t GetLogFileMaxSize() const { return m_log_file_max_size; }
    inline size_t GetLogSyncInterVal() const { return m_log_sync_interval; }
    ~Config() = default;
};


}// rocket

#endif // ROCKET_COMMON_CONFIG_H