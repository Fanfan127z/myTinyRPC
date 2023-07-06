#include "tcp_connection.h"
#include "../fd_event_group.h"
#include "../../common/log.h"
#include <unistd.h> // use read()
#include <error.h> // use errno 
#include <string>
#include <cstring>
#include <vector>
#include "../string_codec.h"
#include "../codec/tiny_pb_codec.h"
#include "../codec/tiny_pb_protocol.h"

namespace rocket{

TcpConnection::TcpConnection(EventLoop* eventloop, int cfd, size_t buffer_size
        , const NetAddrBase::s_ptr& peer_addr, const TcpConnectionType& type /* = TcpConnectionByServer */)
        :m_event_loop(eventloop),m_cfd(cfd),m_connection_type(type)
        {
        // 创建接收和发送缓冲区
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_peer_addr = peer_addr;
        // 拿到可用的fdevent
        m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(cfd);
        m_fd_event->setNonBlock();// set fd为非阻塞的
        
        m_local_addr = std::make_shared<IPv4NetAddr>();
        setState(NotConnected);
        /* 监听套接字的可读事件 */
        if(m_connection_type == TcpConnectionByServer){
            listenRead();
            /*  只有属于服务端的连接时，才需要主动监听可读事件
                if是客户端连接时，只要要读回报时才监听 */
        }
        m_codec = std::make_shared<TinyPbCodec>();
}
void TcpConnection::onRead(){
    // 从 socket缓冲区中，调用系统read函数 读取字节流进入in_buffer中
    if(getState() != Connected){
        // TODO: 这里我直接注释掉！
        // ERRORLOG("TcpConnection::onRead error, client has already disconnected, client addr[%s], client connectfd[%d]",
        //         m_peer_addr->toString().c_str(), m_cfd);
        return;
    }
    // 一直循环读取，只要没有读取完毕就继续读
    bool has_read_all = false;// 代表是否已经读取完毕all的bytes data
    bool peer_is_close = false;// 代表对端client是否已关闭连接
 
    while(!has_read_all){
        // 如果自定义的in_buffer不够空间写入客户端连接触发的可读事件之后，::read()的字节数 就 先行 扩容一下
        if(m_in_buffer->writeAble() <= 0){
            size_t new_size =  2 * m_in_buffer->size();
            m_in_buffer->resizeBuffer(new_size);
            INFOLOG("In onRead, resize in_buffer one time, it's new size is [%d]", new_size);
        }
        // 获取当前的in_buffer能够被read所得写入的字节数
        int read_counts = m_in_buffer->writeAble();// read_counts代表：当前可以 一次性读取的字节数
        int write_idx = m_in_buffer->writeIndex();
        // read read_counts bytes into m_in_buffer from m_fd_event's client 通信fd
        // tmp_read_counts 是临时的 读取了的 字节数

        int tmp_read_counts = ::read(m_cfd, &(m_in_buffer->m_buffer[write_idx]), read_counts);
        if(tmp_read_counts > 0){
            // 打印日志
            INFOLOG("In onRead, success read [%d] bytes from addr[%s], client cfd[%d]"
                , read_counts, m_peer_addr->toString().c_str(), m_cfd);
            // 调整下可写的index
            m_in_buffer->moveWriteIndex(tmp_read_counts);
            if(tmp_read_counts == read_counts){
                /* 此时说明成功read了read_counts的bytes，但此时触发的读缓冲区中 可能 还有可读的数据
                   因此要继续continue来循环读，直到读取完毕为止！ */
                continue;
            } else if(tmp_read_counts < read_counts){
                // 此时说明已经读取完触发read的缓冲区中的all bytes了！
                has_read_all = true;
                break;// 记得退出循环
            }
            // 注：tmp_read_counts比read_counts还大的情况不存在，::read()函数本身就规定了这个点
        } else if(tmp_read_counts == -1){
            /* 此时发送缓冲区已满了，不能再发送了,这种情况下，就等待下一次缓冲区 可写触发的时候再继续发送data即可
                此时，因为是非阻塞读，如果tmp_read_counts == -1 && errno == EAGAIN时，就会报错
                表示 已经写到没有数据可读了 的意思！ */
            if(errno == EAGAIN){
                ERRORLOG("onRead error, create error, tmp_read_counts == -1 and errno == EAGAIN");
                has_read_all = true;
            } else { ERRORLOG("onRead error, errno = [%d], error info = [%s]", errno, strerror(errno)); }

            break;
        } else if(tmp_read_counts == 0){
            peer_is_close = true;// 此时，对端client关闭了连接
            break;// 千万别忘记结束循环
        }
    }// while
    if(peer_is_close){// 对端已关闭连接
        INFOLOG("In onRead, peer closed connection, peer addr[%s], client cfd[%d]"
            , m_peer_addr->toString().c_str(), m_cfd);
        // TODO: do关闭连接的操作(处理关闭的连接)
        clear();
        return;// 对端已关闭连接，无需继续execute()执行任务真正do事情了(即：提前退出onWrite函数，无需do任何事情)
    }
    if(!has_read_all){
        ERRORLOG("In onRead, not read all bytes data");
    }

    // 简单的 echo，后面会补充 RPC 协议的解析操作
    execute();
}

void TcpConnection::execute(){
    //  TODO: 将 RPC请求作为入参，执行业务逻辑得到RPC响应，再把响应发送回去
    // 现在是do测试代码，所以直接echo一下就行了！
    if(m_connection_type == TcpConnectionByServer){
        std::vector<AbstractProtocol::s_ptr> result_msgs;
        std::vector<AbstractProtocol::s_ptr> reply_msgs;
        m_codec->decode(m_in_buffer, result_msgs);
        for(size_t i = 0;i < result_msgs.size();++i){
            // 1.针对 每一个请求，调用 RPC 方法，获取相应msg
            // 2.将相应msg放入到发送缓冲区，并监听可写事件 回包
            INFOLOG("success get request[%s] from client[%s]",
             result_msgs[i]->getRequestId().c_str(), m_peer_addr->toString().c_str());
            std::shared_ptr<TinyPbProtocol> msg = std::make_shared<TinyPbProtocol>();
            msg->m_pb_data = "hello, this is rocket rpc test pb_data";
            msg->setRequestId(result_msgs[i]->getRequestId());
            reply_msgs.emplace_back(msg);
        }
        m_codec->encode(reply_msgs, m_out_buffer);
        listenWrite();// 监听可写事件，当epoll监听可写事情触发时，RPC-server就将out_buffer中的回包写回去给client

    } else if(m_connection_type == TcpConnectionByClient){
        // 从 buffer 里面 decode 得到 message 对象，接着 判断req_id是否相等，相等则读成功，执行其回调函数
        std::vector<rocket::AbstractProtocol::s_ptr> out_msgs;
        m_codec->decode(m_in_buffer, out_msgs);
        
        for(size_t i = 0;i < out_msgs.size();++i){
            std::string req_id = out_msgs[i]->getRequestId();
            auto it = m_read_dones.find(req_id);
            if(it != m_read_dones.end()){
                it->second(out_msgs[i]);// 执行回调函数
                // it->second(out_msgs[i]->shared_from_this());// 执行回调函数，不需要这样吧，因为我本来msgs就是用shared_ptr来管理的了
            }
        }
    }
}

void TcpConnection::onWrite(){
    // 从 socket缓冲区中，调用系统write函数 从out_buffer中发送字节流出去
    // (调用系统write函数)将当前的 out_buffer中的all bytes data 全部发送回去给 client
    if(getState() != Connected){
        ERRORLOG("TcpConnection::onWrite error, client has already disconnected, client addr[%s], client connectfd[%d]",
            m_peer_addr->toString().c_str(), m_cfd);
        return;
    }
    if(m_connection_type == TcpConnectionByClient){
        // 1.将 message encode 得到字节流
        // 2.将 字节流 写入 buffer里面，然后等后面全部发送
        std::vector<AbstractProtocol::s_ptr> msgs;
        for(size_t i = 0;i < m_write_dones.size(); ++i){
            msgs.push_back(m_write_dones[i].first);
        }
        m_codec->encode(msgs, m_out_buffer);
    }
    bool has_write_all = false;// 代表是否已经发送完毕all的bytes data
    bool peer_is_close = false;// 代表对端client是否已关闭连接
    // 一直循环发送，只要没有发送(写)完毕就继续写
    while(!has_write_all){
        // 先判断一下out_buffer是否还有data可以被发送
        if(m_out_buffer->readAble() <= 0){
            INFOLOG("In onWrite, out_buffer has no space to be read, so no data need to be sent to client[%s]"
            , m_peer_addr->toString().c_str());
            has_write_all = true;
            break;
        }
        // 获取当前的out_buffer能够被读取用来write的字节数
        int write_counts = m_out_buffer->readAble();
        int read_idx = m_out_buffer->readIndex();
        
        int tmp_write_counts = ::write(m_cfd, &m_out_buffer->m_buffer[read_idx], write_counts);
        if(tmp_write_counts >= write_counts){
            // INFOLOG("In onWrite, success write [%d] bytes to addr[%s], client cfd[%d]"
            // , write_counts, m_peer_addr->toString().c_str(), m_cfd);
            INFOLOG("In onWrite, no data need to send to client [%s]", m_peer_addr->toString().c_str());
            m_out_buffer->moveReadIndex(tmp_write_counts);
            has_write_all = true;
            break;// 记得退出循环
        } else if(tmp_write_counts == -1){
            /* 此时发送缓冲区已满了，不能再发送了,这种情况下，就等待下一次缓冲区 可写触发的时候再继续发送data即可
               此时，因为是非阻塞写，如果tmp_write_counts == -1 && errno == EAGAIN时，就会报错
               表示 已经写到没有数据可写了 的意思！*/ 
            if(errno == EAGAIN){
                ERRORLOG("onWrite has done, create error, tmp_write_counts == -1 and errno == EAGAIN");
            } else { ERRORLOG("onWrite error, errno = [%d], error info = [%s]", errno, strerror(errno)); }
            break;
        } else if(tmp_write_counts == 0){
            peer_is_close = true;// 此时，对端client关闭了连接
            break;// 千万别忘记结束循环
        }
    }// while
    if(peer_is_close){// 对端已关闭连接
        // TODO: do关闭连接的操作(处理关闭的连接)
        clear();
        INFOLOG("In onWrite, peer closed connection, peer addr[%s], client cfd[%d]"
            , m_peer_addr->toString().c_str(), m_cfd);
        return;// 提前退出onWrite函数，无需do任何事情了！
    }
    if(!has_write_all){
        ERRORLOG("In onWrite, not write all bytes data");
    } else {
        // 发送完一次之后，就取消监听 写事件，不然写事件会被一直触发（我们本意是只让写事件写一次，即只触发一次的意思）
        m_fd_event->cancle_listen(FdEvent::OUT_EVENT);
        m_event_loop->addEpollEvent(m_fd_event.get());// 添加到epoll中，去do真正删除监听的操作
    }
    if(m_connection_type == TcpConnectionByClient){// 发送完之后，执行对应的回调函数
        for(size_t i = 0;i < m_write_dones.size(); ++i){
            m_write_dones[i].second(m_write_dones[i].first);// 一次性 执行完all的回调函数（按顺序）
        }
    }
    m_write_dones.clear();// 执行完之后，就做一个清空操作
}
// 处理一些（来自客户端client主动）关闭连接后的清理工作
void TcpConnection::clear(){
    if(getState() == Closed){
        return;// 已经关闭，无需do任何事情
    }
    // 先把 所监听的读写事件 都取消掉
    m_fd_event->cancle_listen(FdEvent::IN_EVENT);
    m_fd_event->cancle_listen(FdEvent::OUT_EVENT);

    // 然后再将套接字从eventloop在去除！
    // ( 去除之后eventloop就不会监听该fdevent的IO事件, 因为关闭连接了，无需再收发数据，
    // so理所当然无需继续监听fd对应的该fdevent的读/写事件了 )
    m_event_loop->deleteEpollEvent(m_fd_event.get());
    setState(Closed);// set连接的状态，代表此连接以及die了
    // ::close(m_cfd);
}
// 处理服务器端主动关闭连接后的清理工作
void TcpConnection::shutdown(){
    // key思路：借助 系统调用 来触发服务器端的 四次挥手的过程

    if(getState() == Closed || getState() == NotConnected){
        return;// 此时，客户端连接已主动关闭 或 未连接，就无需服务器端主动关闭处理
    }
    // 注意，当服务器端主动断开连接时，此时是处于 TCP的半关闭状态
    /*
        在C-S模型中的TCP连接，半关闭状态发生在一方完成数据发送并发送FIN报文段时。
        这时，该方已经关闭了数据传输，但仍然可以接收对方发送的数据。
    */
    setState(HalfClosing);
    // 调用 系统::shutdown() 关闭服务器端的读写IO，这意味着服务器端不会再对这个fd进行读写IO操作
    // 但客户端仍然可以继续向服务器端进行读写IO操作，因为TCP通信是双端的（C-S模型）
    // 即：发送FIN报文 给客户端，触发了四次挥手的第一个阶段
    // 当fd 触发可读事件，但可读的数据字节数为0时，说明此时对端也发送了FIN报文，代表断开连接了
    int ret = ::shutdown(m_cfd, SHUT_RDWR);
    if(ret == -1){
        ERRORLOG("TcpConnection::shutdown() failed, errno = [%d], error info = [%s]", errno, strerror(errno));
    }
    INFOLOG("success shutdown the tcp-connection");
}

// 启动监听可写事件
void TcpConnection::listenWrite(){
    // 监听客户端通信fd的可写事件，可写了就调用onWrite把RPC响应结果返回客户端
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event.get());// 添加到epoll中
}
// 启动监听可读事件
void TcpConnection::listenRead(){
    // 监听客户端通信fd的可读事件，可读了就调用onWrite把RPC响应结果返回客户端
    /* 注：std::bind(&TcpConnection::onRead, this)中传入this指针表示只调用属于该对象的TcpConnection::onRead成员方法！
        当connectfd发生可读事件时，就会去执行read方法 */
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event.get());// 注册到epoll中！才会真正生效去监听可读事件！千万别忘记了这一步！
}
// push 要发送的Msgs对象
void TcpConnection::pushSendMsg(AbstractProtocol::s_ptr msg
    , const std::function<void(AbstractProtocol::s_ptr)>& callback){
    m_write_dones.push_back({msg, callback});
}
// push 要读取的Msgs对象
void TcpConnection::pushReadMsg(std::string req_id, 
    const std::function<void(AbstractProtocol::s_ptr)>& callback){
    m_read_dones.insert(std::make_pair(req_id, callback));// m_read_dones.insert({req_id, callback});
}
TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection()");
}


}// rocket