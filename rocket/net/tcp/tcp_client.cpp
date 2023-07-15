#include "tcp_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring> // use strerror()
#include "../../common/log.h"
#include "../eventloop.h"
#include "../fd_event_group.h"
#include "../io_thread_group.h"
#include "../../common/error_code.h"
#include <error.h>

namespace rocket{

TcpClient::TcpClient(const NetAddrBase::s_ptr& peer_addr):m_peer_addr(peer_addr){
   // m_local_addr = new IPv4NetAddr()
   m_event_loop = EventLoop::GetCurrentEventLoop();// 获取当前线程的eventloop对象
   m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
   if(m_fd == -1){
      ERRORLOG("TcpClient socket error, errno = [%d], error info = [%s]", errno, strerror(errno));
      exit(-1);
   }
   m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(m_fd).get();
   m_fd_event->setNonBlock();// 下面， TcpConnection类的构造函数中已经set了非阻塞，so这里无需重复set了！
   // 因为
   m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, nullptr, m_peer_addr, TcpConnectionByClient);
   m_connection->setTcpConnectionType(TcpConnectionByClient);// 表示 set 为 是客户端来建立 的连接
   // DEBUGLOG("TcpClient create success");
}
/* 注意，我们所有的连接或读写 connect/write/read 都是 异步 执行的！
   如果connect连接成功，则回调函数done()会被执行！*/
void TcpClient::Connect(std::function<void()> done){// done是回调函数
   int ret = ::connect(m_fd, m_peer_addr->getSocketAddr(), m_peer_addr->getSocketLen());
   if(ret == 0){
      INFOLOG("TcpClient connect Server[%s] success, fd = [%d]", m_peer_addr->toString().c_str(), m_fd);
      InitLocalAddr();
      m_connection->setState(TcpConnection::Connected);
      if(done){
         done();// 执行回调函数
         DEBUGLOG("TcpClient callback function has been called");
      } 
   } else if(ret == -1){
      if(errno == EINPROGRESS){
         // 添加 可写事件 给 epoll 监听 ，并判断错误码
         auto callback = [this, done](){
            int ret = ::connect(m_fd, m_peer_addr->getSocketAddr(), m_peer_addr->getSocketLen());
            if((ret < 0 && errno == EISCONN) || (ret == 0)){
               INFOLOG("TcpClient connect Server[%s] success, fd = [%d]", m_peer_addr->toString().c_str(), m_fd);
               InitLocalAddr();
               m_connection->setState(TcpConnection::Connected);
            } else {
               if(errno == ECONNREFUSED){
                  this->m_connect_error_code = ERROR_PEER_CLOSED;
                  this->m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
               } else {
                  this->m_connect_error_code = ERROR_FAILED_CONNECT;
                  this->m_connect_error_info = "connect error, sys error = " + std::string(strerror(errno));
               }
               ERRORLOG("TcpClient connect error, errno = [%d], error info = [%s]", errno, strerror(errno));
               // 连接失败之后，需要关闭原来的套接字，并重新申请一个，以便于后续在此connect
               ::close(m_fd);
               m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
            }
            // 连接完成之后，需要去掉可写事件的监听，不然服务端会一直触发。
            // m_fd_event->cancle_listen(FdEvent::OUT_EVENT);
            // m_event_loop->addEpollEvent(m_fd_event);
            m_event_loop->deleteEpollEvent(m_fd_event);
            if(done){ // 如果 回调函数非空，才 执行回调函数
               done();
               DEBUGLOG("TcpClient callback function has been called");
            }
         };
         // auto error_callback = [this, done](){
         //    if(errno == ECONNREFUSED){
         //       this->m_connect_error_code = ERROR_FAILED_CONNECT;
         //       this->m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
         //    }else{
         //       this->m_connect_error_code = ERROR_FAILED_CONNECT;
         //       this->m_connect_error_info = "connect unknown error, sys error = " + std::string(strerror(errno));
         //    }
         //    ERRORLOG("TcpClient connect error, errno = [%d], error info = [%s]", errno, strerror(errno));
         // };
         // m_fd_event->listen(FdEvent::OUT_EVENT, callback, error_callback);
         m_fd_event->listen(FdEvent::OUT_EVENT, callback);
         // 监听其可写事件
         m_event_loop->addEpollEvent(m_fd_event);
         if(!m_event_loop->is_looping())m_event_loop->loop();
      } else {
         ERRORLOG("TcpClient connect error, errno = [%d], error info = [%s]", errno, strerror(errno));
         this->m_connect_error_code = ERROR_FAILED_CONNECT;
         this->m_connect_error_info = "connect error, sys error = " + std::string(strerror(errno));
         if(done){ // 如果 回调函数非空，才 执行回调函数
            done();
            DEBUGLOG("TcpClient callback function has been called");
         }
      }
   }// if end
}// Connect

/* 异步地 发送 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
   如果write发送成功，会调用 done函数，函数的入参就是message对象 */
void TcpClient::Write(AbstractProtocol::s_ptr msg, std::function<void(AbstractProtocol::s_ptr)> done){
   // 逻辑：
   /* 1. 把 要发送的msg 对象和 done回调函数 写入到 Connection 的buffer中（connection对象的onWrite方法已经帮我们do好了）
      2. 让 connection 启动 监听 可写事件 */ 
  m_connection->pushSendMsg(msg, done);
  m_connection->listenWrite();
}
/* 异步地 读取 message，这里的message是个广义的对象，可以是字符串，也可以是我们后面定义好的rpc对象
   如果read读取成功，会调用 done函数，函数的入参就是message对象 */
void TcpClient::Read(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done){
   // 逻辑：
   /* 1.监听可读事件
      2.从 buffer 里面 decode 得到 message 对象，接着 判断msg_id是否相等，相等则读成功，执行其回调函数 */
   m_connection->pushReadMsg(msg_id, done);// push可读的msg进去，注册回调函数done
   m_connection->listenRead();// 然后开启 监听 可读事件
}
// 结束客户端的loop死循环
void TcpClient::Stop(){
   if(m_event_loop->is_looping()){
      m_event_loop->stop();
   }
   DEBUGLOG("TcpClient loop end");
}
// 初始化本地客户端的网络地址
void TcpClient::InitLocalAddr(){
   struct sockaddr_in local_addr;
   socklen_t len = sizeof(local_addr);
   int ret = getsockname(m_fd, (struct sockaddr*)(&local_addr), &len);
   if(ret != 0){
      ERRORLOG("InitLocalAddr error, errno[%d], error info[%s]", errno, strerror(errno));
      return;
   }
   m_local_addr = std::make_shared<IPv4NetAddr>(local_addr);
}
// 添加监听定时器任务
void TcpClient::AddTimerEvent(TimerEvent::s_ptr timerEvent){
   m_event_loop->addTimerEvent(timerEvent);
}
// // 取消监听定时器任务
// void TcpClient::CancleTimerEvent(TimerEvent::s_ptr timerEvent){
//    m_event_loop->deleteEpollEvent();
// }
TcpClient::~TcpClient(){
   if(m_fd > 0){
      close(m_fd);
   }
   INFOLOG("~TcpClient()");
}

}// rocket