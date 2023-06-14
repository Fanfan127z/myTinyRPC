###  project record


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