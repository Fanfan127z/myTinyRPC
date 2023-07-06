#include "tiny_pb_protocol.h"

namespace rocket{

/* 静态数据成员必须是，类内定义，类外初始化，
或者在类的.cc/.cpp文件中去实现即可，但是，一定不能在类的头文件中去初始化静态的成员变量！*/
char TinyPbProtocol::PB_START = 0x02;
char TinyPbProtocol::PB_END = 0x03;



}// rocket