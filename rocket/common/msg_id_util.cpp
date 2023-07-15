#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "msg_id_util.h"
namespace rocket{
static int g_msg_id_length = 20;
static int g_random_fd = -1;
static thread_local std::string t_msg_id_num;// 记录当前线程的msg_id的值
static thread_local std::string t_max_msg_id_num;// 记录当前线程的msg_id的最大值

// 这个随机生成MsgId的函数 的算法思路，你怎么设置都行，反正合理设置即可了！
std::string MsgIdUtil::GenerateMsgId(){
    if(t_msg_id_num.empty() || t_msg_id_num == t_max_msg_id_num){
        // 说明当前线程的msg_id的值不可以递增了
        if(g_random_fd == -1){
            g_random_fd = open("/dev/urandom", O_RDONLY);// 以可读的方式，打开这个路径/dev/urandom的urandom文件
        }
        std::string res(g_msg_id_length, 0);
        if( (read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length){
            ERRORLOG("read from /dev/urandom error");
            return "";
        }
        for(int i = 0;i < g_msg_id_length;++i){
            uint8_t x = ( (uint8_t)(res[i]) ) % 10;
            res[i] = x + '0';
        }
        t_msg_id_num = res;
    } else {
        size_t i = t_msg_id_num.length() - 1;
        while(t_msg_id_num[i] == '9' && i >= 0)i--;
        if(i >= 0){
            t_msg_id_num[i] += 1;
            for(size_t j = i + 1;j < t_msg_id_num.length();++j){
                t_msg_id_num[j] = '0';
            }
        }
    }
    return t_msg_id_num;
}
MsgIdUtil::MsgIdUtil(){

}
MsgIdUtil::~MsgIdUtil(){
    
}



}// rocket