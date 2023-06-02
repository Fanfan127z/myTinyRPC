#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <string> // 存放 配置变量
namespace rocket{

class Config{
private:
    // 配置变量 们
    std::string m_log_level;


public:
    Config() = default;
    Config(const char * xmlFilePath);
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char * xmlFile);
    inline std::string getLogLevel(){ return this->m_log_level; }
    ~Config() = default;
};


}// rocket

#endif // ROCKET_COMMON_CONFIG_H