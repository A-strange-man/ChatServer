#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    
    void start();

private:
    // 连接创建/断开 的回调函数
    void onConnection(const TcpConnectionPtr & conn);
    // 读写事件的回调函数
    void onMessage(const TcpConnectionPtr & conn, Buffer * buffer, Timestamp time);

private: 
    TcpServer m_server;
    EventLoop* m_loop;
};