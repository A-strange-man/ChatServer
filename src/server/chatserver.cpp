#include "chatserver.h"
#include "json.hpp"
#include "chatservice.h"

#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
            : m_server(loop, listenAddr, nameArg)
            , m_loop(loop)
{
    // 注册连接创建/断开 的回调函数
    m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    
    // 注册消息回调函数
    m_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    m_server.setThreadNum(4);
}

void ChatServer::start()
{
    m_server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr & conn)
{
    if (!conn->connected())     //不是建立连接，则是客户端断开连接
    {
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr & conn, Buffer* buffer, Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    // 数据反序列化
    json js = json::parse(buf);

    auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
    
    msgHandler(conn, buf, time);
}
