##  project record


why要有这个定时器呢？因为，一个RPC任务执行之后，我们不可能等待很长的时间
也不可能一直等待下去，如果超过一定的时间之后，如果还没有返回我们想要调用的服务的信息，那就需要记录日志描述这次的RPC调用的错误了！此时就需要引入(do一个)这个定时器！

#### 2.4.1 TimeEvent 定时任务 事件
1. 指定任务的到达时间点 arrive_time
2. 到达下一次任务执行的时间间隔 interval, 单位是ms毫秒
3. 判断该任务是否是需要周期性执行的任务 is_repeated，比如一个任务我们反复执行10次，时间间隔是5ms
4. 判断是否取消该定时器的标志 is_cancled
5. task

方法:
cancle(): 取消一个定时任务
cancleRepeated(): 一个定时任务原来是重复执行的，但现在我们想给他取消重复执行这个熟属性就这么干！

#### 2.4.2 Timer 定时器
    Timer 是一个 TimerEvent 的集合
    Timer 继承自 FdEvent
    
    方法：
    addTimerEvent();
    deleteTimerEvent();
    onTimer();
    /*
        当Timer所绑定的fd被epoll_wait检测到有IO事件触发(发生)了之后，就需要执行任务函数
        那么这个任务函数(或者说方法就是onTimer)
    */
    
    reserArriveTime();
    multimap<key(arrivetime), TimerEvent> : 按照到达时间arrivetime来存储管理我们的TimerEvent， 因为到达时间arrivetime有可能重复（同一个时间执行多个任务），这是复合逻辑的！so不用map来管理！



#### 2.5 IO 线程
'''
创建一个新IO线程 要do的工作是：
1.创建一个新线程（pthread_create）
2.在新线程里面 创建一个EventLoop，并完成初始化
3.开启loop循环监听读写事件，并do（调用）对应的任务（回调函数）
class {

    pthread_t m_thread;// 指向当前的线程
    pid_t m_thread_id;// 当前线程的id
    EventLoop m_event_loop;// 当前线程所持有的epoll事件Loop的对象
}
'''

###  TcpBuffer
为什么需要应用层Buffer？
1：方便数据处理，特别是应用层的包组装和拆解
2：方便异步的发送（发送数据直接塞到发送缓冲区里面，等待epoll异步发送）
3：提高发送效率，多个包合并一起发送
总体上，提高网络包字节流的传输效率

### TcpAccepter

设置端口复用的原因:
TCP/IP的C-S（客户端-服务器）通信模型中，端口复用主要是为了提高网络资源的利用率。在TCP/IP协议中，一个完整的连接需要四个元素：源IP地址、源端口、目标IP地址和目标端口。这四个元素共同组成了一个唯一的连接，我们称之为套接字（Socket）。

端口复用的主要原因有以下几点：

资源有限：在TCP/IP协议中，端口号是一个16位的数字，也就是说最多有65536个端口。但实际上，1-1024号端口已经被系统预留，只有1025-65535号端口可供应用程序使用。如果不允许端口复用，那么同时进行的连接数将受到严重限制。

提高效率：如果不允许端口复用，那么每次连接断开后，该端口需要经过一个TIME_WAIT阶段，大约需要等待2分钟才能再次使用。这样会导致大量的端口处于等待状态，无法立即重复使用，降低了系统的效率。

并发处理：在高并发的情况下，如果不允许端口复用，那么可能会出现端口不够用的情况。而端口复用可以让一个端口服务于多个连接，从而提高并发处理的能力。

如果不设置端口复用，可能会导致以下问题：

端口耗尽：如果连接数过多，可能会出现可用端口不足的情况，导致新的连接无法建立。

系统效率降低：如上所述，每个端口在使用后需要经过一个TIME_WAIT阶段，这会导致大量的端口无法立即重复使用，降低了系统的效率。

并发处理能力降低：如果不允许端口复用，那么一个端口只能服务于一个连接，这会降低系统的并发处理能力。

### TcpServer

mainReactor由主线程运行，其作用如下：通过epoll监听listenfd的可读事件，当可读事件发生之后，调用accept函数
获取connectfd，然后随机取出一个subReactor，将conncetfd的读写事件注册到这个subReactor的epoll上即可。
mainReactor只负责建立连接事件，不进行业务处理，也不关心已连接套接字的IO事件。
subReactor通常有多个，没有给subReactor由一个线程来运行，其注册conncectfd的读写事件，当发生IO事件后
需要进行业务处理。

### TcpClient

方法：
1.Connect(建立连接)：连接 对端 机器
2.Write(发送数据)：将 RPC响应 发送给 客户端
3.Read(等待回包)：读取 客户端 发来的数据，组装为 RPC请求

这个类的key点在于非阻塞的Connect的使用，
返回0标识连接成功
返回-1，但是errno == EINPROGRESS,表示连接正在建立，此时将套接字添加（绑定）到epoll中去监听其
可写事件。等待可写事件就绪后，调用getsockopt获得fd上的错误，错误为0代表连接建立成功。
其他errno直接报错。
bash: man 2 connect 查看如下
EINPROGRESS
    The  socket is nonblocking and the connection cannot be completed immediately.  (UNIX domain sockets failed with EAGAIN instead.)  It is possible to select(2) or
    poll(2) for completion by selecting the socket for writing.  After select(2) indicates writability, use getsockopt(2)  to  read  the  SO_ERROR  option  at  level
    SOL_SOCKET  to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, ex‐
    plaining the reason for the failure).

EAGAIN For nonblocking UNIX domain sockets, the socket is nonblocking, and the connection cannot be completed immediately.  For other socket families, there are  insuf‐
        ficient entries in the routing cache.



### RPC-Codec

why自定义协议格式？
既然用了Protobuf做序列化，那为什么不直接把序列化后的结果直接发送，而是在这上面自定义一些字段呢？
1：为了方便分割请求：因为protobuf后的结果是一串无意义的字节流，你无法区分哪里开始或者结束。比如说把两个Message对象序列化后的结果排在一起，你甚至无法分开这两个请求。因为在TCP传输层，数据是按照字节流来传输的，并没有包的概念，因此应用层就更加无法去区分了。

2：为了定位：加上MsgID(req_id)等信息，能够帮助我们匹配一次RPC的请求和响应，不会串包
MsgID是RPC请求的唯一标识，一次RPC的请求和响应的MsgID应当一致。

3：错误提升：加上错误信息，能够很容易知道RPC失败的原因，方便bug问题的定位，而不用说再回到日志去分析bug的出现原因。（比如说我们的服务端根本没有启动服务）

TinyPB协议（Tiny Protobuf Protocol，是轻量化的Protobuf协议）

通过这种协议的开始符（即ASCII码0x02）和结束符（即ASCII码0x03）就能够区分出这是一个RPC包

Protobuf序列化的数据其实就是我们的message对象使用protobuf库序列化出来得到的一串字节流数据。

同时，我还借鉴了TCP协议，使用校验和，对整个RPC请求包进行校验，用于防止包的内容被篡改（校验算法还待定，可自行扩充扩展自定义）

注意：我们所有的整数都是使用网络字节序（大端存储）的方式的。

我写encode和decode的时候，可能有些问题，目前我排查到的就是decode的那些parse success debug logs 没有打印出来！
so 我 明天要著重看一下 decode的代码！

调用TcpConnection::onRead()函数一直有bug，循环调用onRead的 bug，我明天再看看！

ok,原来是 encodeTinyPb的返回值出问题了，必须要返回过去之后再delete，不能用shared_ptr来管理

### RPC-Module

#### RPC过程
                    (request)       (response)
[read] ----> [decode] ----> [dispatch] ----> [encode] ----> [write]
