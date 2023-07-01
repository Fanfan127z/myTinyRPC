#include "tcp_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring> // use strerror()
#include "../../common/log.h"
#include "../eventloop.h"
#include "../fd_event_group.h"
#include "../io_thread_group.h"
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
   // m_fd_event->setNonBlock();// 下面， TcpConnection类的构造函数中已经set了非阻塞，so这里无需重复set了！
   m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(m_fd).get();
   m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, m_peer_addr, TcpConnectionByClient);
   m_connection->setTcpConnectionType(TcpConnectionByClient);// set为 客户端来建立 的连接
   DEBUGLOG("TcpClient create success");
}
/* 注意，我们所有的连接或读写 connect/write/read 都是 异步 执行的！
   如果connect连接成功，则回调函数done()会被执行！*/
void TcpClient::Connect(const std::function<void()>& done){// done是回调函数
   int ret = ::connect(m_fd, m_peer_addr->getSocketAddr(), m_peer_addr->getSocketLen());
   if(ret == 0){
      INFOLOG("TcpClient connect Server[%s] success, fd = [%d]", m_peer_addr->toString().c_str(), m_fd);
      m_connection->setState(TcpConnection::Connected);
      if(!done){
         DEBUGLOG("TcpClient callback function is null");
      } else done();// 执行回调函数
   } else if(ret == -1){
      if(errno == EINPROGRESS){
         // 添加 可写事件 给 epoll 监听 ，并判断错误码
         m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
            int error = 0;
            socklen_t error_len = sizeof(error);
            getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
            bool is_connect_success = false;
            if(error == 0){
               INFOLOG("TcpClient connect Server[%s] success, fd = [%d]", m_peer_addr->toString().c_str(), m_fd);
               is_connect_success = true;
               m_connection->setState(TcpConnection::Connected);
            } else {
               ERRORLOG("TcpClient connect error, errno = [%d], error info = [%s]", errno, strerror(errno));
            }
            // 连接完成之后，需要去掉可写事件的监听，不然服务端会一直触发。
            m_fd_event->cancle_listen(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
            // 如果连接成功 且 回调函数非空，才 执行回调函数
            if(is_connect_success && done){
               done();
               DEBUGLOG("called TcpClient callback function");
            }
         });// 监听其可写事件
         m_event_loop->addEpollEvent(m_fd_event);
         if(!m_event_loop->is_looping())m_event_loop->loop();
      } else {
         ERRORLOG("TcpClient connect error, errno = [%d], error info = [%s]", errno, strerror(errno));
      }
   }
   
}
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
void TcpClient::Read(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done){
   // 逻辑：
   /* 1.监听可读事件
      2.从 buffer 里面 decode 得到 message 对象，接着 判断req_id是否相等，相等则读成功，执行其回调函数 */
   m_connection->pushReadMsg(req_id, done);// push可读的msg进去，注册回调函数done
   m_connection->listenRead();// 然后开启 监听 可读事件
}

TcpClient::~TcpClient(){
   if(m_fd > 0){
      close(m_fd);
   }
   INFOLOG("~TcpClient()");
}

}// rocket