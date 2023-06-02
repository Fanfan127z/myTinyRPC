#include <tinyxml/tinyxml.h>
#include "../common/config.h"
// #include <iostream>

// 使用这个宏替换来 find 到对应配置xml文件中的node
#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name);\
if( ! (name##_node) ){ \
    printf("Start rocket server error, failed to read node [%s].\n", #name);\
    exit(-1); \
} \

// 使用这个宏替换来 find 到对应配置xml文件中的node的孩子node对应的string值
#define READ_STR_FROM_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if( !(name##_node) || !(name##_node->GetText()) ){ \
    printf("Start rocket server error, failed to read node [%s].\n", #name); \
    exit(-1); \
} \
std::string name##_str = std::string(name##_node->GetText());\

namespace rocket {
    static Config* g_config = nullptr;
    Config* Config::GetGlobalConfig(){// static
        if(g_config != nullptr)return g_config;
        g_config = new Config();
        return g_config;
    }
    void Config::SetGlobalConfig(const char * xmlFile){// static
        if(g_config == nullptr){
            g_config = new Config(xmlFile);
        }
    }
    Config::Config(const char * xmlFile){
        TiXmlDocument* xml_document = new TiXmlDocument();
        bool ret = xml_document->LoadFile(xmlFile);
        if(!ret){
            printf("Start rocket server error, failed to read config file [%s], error info [%s].\n", xmlFile, xml_document->ErrorDesc());
            exit(-1);// 标识程序启动失败，因为配置文件就是拿来启动 rpc程序的！
        }
        READ_XML_NODE(root, xml_document);
        READ_XML_NODE(log, root_node);
        READ_STR_FROM_XML_NODE(log_level, log_node);
        this->m_log_level = log_level_str;
    }
} // rocket
