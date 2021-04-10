# ChatServer
可以工作在nginx TCP负载均衡环境中的集群聊天服务器，基于muduo、redis、mysql实现。附客户端代码。



编译方式：

cd build/

rm -rf *

cmake ..

make



## 1. 项目结构

- ChatServer
  - bin 目录：存放可执行文件
  - build 目录：CMake生成的文件
  - include 目录：存放头文件
    - include/server：存放服务器端程序的头文件
    - include/client：存放客户端程序的头文件
  - src目录：存放源码
    - src/server：存放服务器端代码
    - src/client：存放客户端代码
  - thirdparty：第三方库



网络模块：使用muduo网络库。

负载均衡：nginx的TCP负载均衡。1:1按权重配比。

跨服务器通信：redis发布--订阅消息队列。



## 2. 解耦网络模块和业务模块

方案：

利用事件回调。

- 根据 json["msgid"] 获取对应的 事件处理器。
- 一种服务类型对应一个处理器。
- 使用map绑定：unordered_map<int, MsgHandler>



约定 消息类型及其id：

```c++
enum enMsgType
{
    LOGIN_MSG        = 1,        // 登陆消息
    LOGIN_MSG_ACK    = 2,        // 登陆响应消息
    REG_MSG          = 3,        // 注册消息
    REG_MSG_ACK      = 4,        // 注册响应消息
    ONE_CHAT_MSG     = 5,        // 聊天消息
    ADD_FRIEND_MSG   = 6,        // 添加好友消息
    // ADD_FRIEND_MSG_ACK
    CREATE_GROUP_MSG = 7,        // 创建群组
    ADD_GROUP_MSG    = 8,        // 添加群组
    GROUP_CHAT_MSG   = 9,        // 群聊
};
```



## 3. 数据表设计

**User表：**

| 字段名称 | 字段类型                 | 字段说明     | 约束                        |
| -------- | ------------------------ | ------------ | --------------------------- |
| id       | int(11)                  | 用户id       | auto_increment，primary key |
| name     | varchar(50)              | 用户名       | not null,  unique           |
| password | varchar(50)              | 用户密码     | not null                    |
| state    | enum('online','offline') | 当前登陆状态 | default 'offline'           |



**Friend表：**

| 字段名称 | 字段类型 | 字段说明 | 约束                |
| -------- | -------- | -------- | ------------------- |
| userid   | int(11)  | 用户id   | not null,  联合主键 |
| friendid | int(11)  | 好友id   | not null,  联合主键 |



**AllGroup表：**

| 字段名称  | 字段类型     | 字段说明   | 约束                         |
| --------- | ------------ | ---------- | ---------------------------- |
| id        | int(11)      | 组id       | primary key,  auto_increment |
| groupname | varchar(50)  | 组名称     | not null,  unique            |
| groupdesc | varchar(200) | 组功能描述 | default ' '                  |



**GroupUser表：**

| 字段名称  | 字段类型                 | 字段说明 | 约束                |
| --------- | ------------------------ | -------- | ------------------- |
| groupid   | int(11)                  | 组id     | not null            |
| userid    | int(11)                  | 组员id   | not null,  联合主键 |
| grouprole | enum('creator','normal') | 组内角色 | default ‘normal’    |



**OfflineMessage表：**

| 字段名称 | 字段类型     | 字段说明                  | 约束     |
| -------- | ------------ | ------------------------- | -------- |
| userid   | int(11)      | 用户id                    | not null |
| message  | varchar(500) | 离线消息 (存储JSON字符串) | not null |



## 4. 服务类型

- 注册
- 登陆
- 添加好友
- 一对一聊天
- 创建群聊
- 添加群聊
- 群发消息



## 5. 负载均衡

nginx :

```s
stream {
    upstream MyServer {
        server 127.0.0.1:8001 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:8002 weight=1 max_fails=3 fail_timeout=30s;
    }

    server {
        proxy_connect_timeout 1s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```



## 6. 跨服务器通信

使用 redis 发布---订阅消息队列。

- 一台服务器与redis-server建立连接后，开辟一个线程专门接受消息，避免服务线程订阅后阻塞等待。

