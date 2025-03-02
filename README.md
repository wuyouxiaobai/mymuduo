## 异步写日志问题
？？？？？？？？？？？？？？？？？？？？？？

## 水位线
highWaterMarkCallback_（消息积压的回调）
用户使用send发送消息时，如果（oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_）即剩余未发送的数据超过了水位线，
就会调用highWaterMarkCallback_处理，这个回调函数由用户实现并通过Tcpconnection的setHighWaterMarkCallback绑定

## 函数绑定时机Tcpconnection
- Tcpconnection创建时绑定Channel的回调函数
- handleRead回调将读事件的消息写入缓冲inputBuffer_，
- handleWrite回调通过写事件触发，将缓冲outputBuffer_的数据发送出去
- handleClose回调通过关闭事件触发，关闭连接

- Tcpconnection提供send用来向对端发送消息 --》由sendInLoop先发送，如果没有发送完成，会加入到outputBuffer_，并注册到写事件，后序交给handleWrite
- 用户定义onConnection(const TcpConnectionPtr& conn)函数来处理连接事件
- 用户定义onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)函数来处理outputBuffer_的消息

- Tcpconnection通过connectEstablished建立连接，通过connectDestroyed销毁连接

## EventLoop
- wakeupChannel_能够用来唤醒阻塞的SubReactor
- 一个subReactor调用另一个subreactor的quit时，会通过wakeup（）唤醒对应线程执行退出操作
- 一个subReactor在使用另一个subreactor的runInLoop处理回调函数时，会通过queueInLoop将对应回调函数加入到另一个subreactor的loop的函数队列中，然后用wakeup（）唤醒线程执行函数队列中的所有函数。

## TcpServer
- 管理
- loop_主Reactor
- acceptor_
- threadPool_ （线程池管理loops、threads）
- start启动线程池（threadPool_）和主loop_（通过acceptor_监听并分发链接）
- 提供setThreadInitCallback、setConnectionCallback、setMessageCallback、setWriteCompleteCallback设置Tcpconnection的对应回调

- TcpConnection通过TcpServer的newConnection，绑定到acceptor_。newConnection通过轮询创建新连接（绑定分配的loop，创建对应的channel，绑定setThreadInitCallback、setConnectionCallback、setMessageCallback、setWriteCompleteCallback这些设置的回调函数）
- 链接创建过程，Tcpserver--》Tcpconnnection--》Channel--》绑定EventLoop

- channel调用TcpConnection绑定handleClose，进而调用Tcpserver绑定到Tcpconnection的removeConnection，然后通过runInLoop在loop中注册删除连接的回调removeConnectionInLoop然后删除连接
- 链接销毁过程，客户端断开连接触发Channel--》TcpConnection--》Tcpserver--》依次进行TcpConnection从connections_删除（因为使用shared_ptr，所以此时如果没有channel使用这个连接，那么这个连接就释放了内存）--》Channel的析构。
- 如果将Tcpconnection从connections_ 删除时对应的Channel还在handleEvent，那么将会通过std::shared_ptr<void> guard = tie_.lock()将Tcpconnection的生命延长到handleEvent完成。

## muduo 使用方法

### 1. 创建 TcpServer
首先，你需要创建一个 `TcpServer` 对象来监听指定的端口并处理客户端连接。以下是一个简单的示例：


### 2. 处理连接和消息
在 `muduo` 中，你可以通过设置回调函数来处理客户端连接和接收到的消息。以下是一个示例：
void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
                << conn->peerAddress().toIpPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time.toString();
    conn->send(msg);
}

### 3. 多线程支持
`muduo` 支持多线程模型，可以通过设置 `EventLoopThreadPool` 来实现多线程处理连接和消息。以下是一个示例：
server.setThreadNum(4); // 设置 4 个子线程

### 4. 发送数据
你可以通过 `TcpConnection` 的 `send` 方法向客户端发送数据。以下是一个示例：
conn->send("Hello, client!");

### 5. 关闭连接
你可以通过 `TcpConnection` 的 `shutdown` 方法关闭连接。以下是一个示例：
conn->shutdown();


### 6. 日志记录
`muduo` 提供了日志记录功能，可以通过 `Logger` 类来记录日志。以下是一个示例：
LOG_INFO("This is an info message");
LOG_ERROR("This is an error message");

### 7. 事件循环
`muduo` 的核心是事件循环，你可以通过 `EventLoop` 类来管理事件循环。以下是一个示例：
EventLoop loop;
loop.loop();