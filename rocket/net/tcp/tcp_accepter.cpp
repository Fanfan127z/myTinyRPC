#include "tcp_accepter.h"
#include "../../common/log.h"// use log
#include <error.h>
#include <string.h>// use memset
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace rocket{
TcpAccepter::TcpAccepter(const NetAddrBase::s_ptr& netaddr):m_local_addr(netaddr){
    if(!m_local_addr->checkValid()){
        ERRORLOG("Server TcpAccepter failed, netaddr[%s] is invalid",m_local_addr->toString().c_str());
        exit(-1);// 程序异常退出
    }
    m_family = m_local_addr->getFamily();
        // 1. 创建监听的套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1){// 服务端套接字都没法创建成功肯定没法正常往下do事情了！
        DEBUGLOG("Server TcpAccepter socket error, errno = [%d], error info = [%s]", errno, strerror(errno));
        exit(-1);// 程序异常退出
    }
    m_listenfd = lfd;
    // 设置端口复用(端口可被重复利用，非必须)
    int opt = 1;
    if(setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        ERRORLOG("setsockopt REUSEADDR error, errno = [%d], error info = [%s]", errno, strerror(errno));
        // 因为是非必要的，因此此时程序不退出
    }

    // 设置fd为非阻塞，
    // 讲文件描述符fd设置为非阻塞的，因为我们的主从reactor模型是不允许阻塞读/写的，读写都是异步的
    // int flag = fcntl(m_listenfd, F_GETFL);// 得到文件描述符的属性
    // flag |= O_NONBLOCK;
    // fcntl(m_listenfd, F_SETFL, flag);

    // 2.将socket()返回值与本地server的IP和端口绑定到一起
    int ret = bind(m_listenfd, m_local_addr->getSocketAddr(), m_local_addr->getSocketLen());
    if(ret == -1){
        DEBUGLOG("Server TcpAccepter bind error, errno = [%d], error info = [%s]", errno, strerror(errno));
        exit(-1);// 程序异常退出
    }
    // 3.设置监听
    ret = listen(m_listenfd, 1000);
    if(ret == -1){
        DEBUGLOG("Server TcpAccepter listen error, errno = [%d], error info = [%s]", errno, strerror(errno));
        exit(-1);// 程序异常退出
    }
}

TcpAccepter::~TcpAccepter(){
    // 析构时，重新 初始化 各个成员变量为默认值
    // memset(&m_client_addr, 0, sizeof(m_client_addr));
    // m_client_addr_len = {0};
    m_family = {-1};
    m_listenfd = {-1};
    m_connectfd = {-1};
    INFOLOG("~TcpAccepter()");
}
std::pair<int, NetAddrBase::s_ptr> TcpAccepter::accept(){
    // 4.阻塞等待并接受客户端的连接
    // 注意：这里必须先设置client_addr_len为原本的结构体大小，而不是0
    // 因为accept中的client_addr_len是传入传出参数，不这么do无法传出正确的client_addr的真实值
    std::pair<int, NetAddrBase::s_ptr> ret_pair;
    if(m_family == AF_INET){// IPv4
        // 所监听客户端的socket_addr
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        memset(&client_addr, 0, client_addr_len);// 初始化一下，防止数据有问题！
        int cfd = ::accept(m_listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(cfd == -1){
            DEBUGLOG("TcpAccepter accept error, error = [%d], error info = [%s]", errno, strerror(errno));
            // 不需要程序异常退出，可以让客户端重新尝试！
        }
        INFOLOG("Server successfuly accepted a client[%s]", IPv4NetAddr(client_addr).toString().c_str());
        ret_pair = std::make_pair(cfd, std::make_shared<IPv4NetAddr>(client_addr));
    }
    else {
        //...其他协议也类似，同上理，可扩展
        return {-1, nullptr};
    }
    return ret_pair;
}
}// rocket