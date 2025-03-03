# my_muduo 项目文档

## 1. 核心概念

### 1.1 异步写日志
待补充...

### 1.2 水位线机制
- `highWaterMarkCallback_`：消息积压回调
  - 当用户使用 `send` 发送消息时，如果满足以下条件：
    - 剩余未发送数据超过水位线 (`oldLen + remaining >= highWaterMark_`)
    - 之前的数据量未超过水位线 (`oldLen < highWaterMark_`)
    - 回调函数已设置 (`highWaterMarkCallback_`)
  - 则会调用 `highWaterMarkCallback_` 处理积压消息

## 2. TcpConnection 详解

### 2.1 回调函数绑定时机
- 创建时：绑定 Channel 的回调函数
- 读事件：`handleRead` 将数据写入 `inputBuffer_`
- 写事件：`handleWrite` 发送 `outputBuffer_` 中的数据
- 关闭事件：`handleClose` 关闭连接

### 2.2 主要功能
- `send`：向对端发送消息
  - 通过 `sendInLoop` 先尝试发送
  - 未发送完成的数据加入 `outputBuffer_` 并注册写事件
  - 后续由 `handleWrite` 处理
- `connectEstablished`：建立连接
- `connectDestroyed`：销毁连接

### 2.3 用户自定义回调
- `onConnection(const TcpConnectionPtr& conn)`：处理连接事件
- `onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)`：处理消息

## 3. EventLoop 机制

- `wakeupChannel_`：用于唤醒阻塞的 SubReactor
- 跨线程操作：
  - `quit()`：通过 `wakeup()` 唤醒线程执行退出
  - `runInLoop`：通过 `queueInLoop` 将回调加入目标线程队列，并唤醒执行

## 4. TcpServer 架构

### 4.1 核心组件
- `loop_`：主 Reactor
- `acceptor_`：监听并分发连接
- `threadPool_`：管理线程池（loops、threads）

### 4.2 主要功能
- `start`：启动线程池和主 loop
- 回调设置：
  - `setThreadInitCallback`
  - `setConnectionCallback`
  - `setMessageCallback`
  - `setWriteCompleteCallback`

### 4.3 连接生命周期管理
- 创建过程：`TcpServer -> TcpConnection -> Channel -> EventLoop`
- 销毁过程：`Channel -> TcpConnection -> TcpServer`
  - 通过 `removeConnection` 和 `removeConnectionInLoop` 安全删除连接
  - 使用 `shared_ptr` 管理连接生命周期

## 5. 使用指南

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