#pragma once

#include "json.hpp"
#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"

#include <unordered_map>
#include <functional>
#include <string>
#include <mutex>
#include <muduo/net/TcpConnection.h>
using namespace std;
using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 消息处理的回调    第二个参数由于编译器版本的原因使用string，没有使用json类型。否则gcc4.8.5会报错
using MsgHandler = std::function<void(const TcpConnectionPtr & conn, string & js, Timestamp time)>;

// 服务类（单例）
class ChatService
{
public:
    static ChatService* getInstance();

    // 登陆
    void login(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 注册
    void reg(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 用户退出
    void loginOut(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 添加好友
    void addFriend(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 创建群聊
    void createGroup(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 添加群聊
    void addgroup(const TcpConnectionPtr & conn, string & js, Timestamp time);
    // 群发消息
    void groupChat(const TcpConnectionPtr & conn, string & js, Timestamp time);
    
    // 客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 服务器退出,重置用户登陆状态
    void reset();

    // 获取消息处理器
    MsgHandler getHandler(int msgid);

    // 处理redis-server上报的消息
    void handleRedisSubscribeMessage(int channel, string msg);

private:
    ChatService();

    // 存储消息id 和 其对应的业务处理方法
    unordered_map<int, MsgHandler> m_MsgHandlerMap;

    // 存储在线用户的连接  m_userConnMap会在多线程环境中修改，需要注意线程安全问题
    unordered_map<int, TcpConnectionPtr> m_userConnMap;

    mutex m_connMutex;

    UserModel m_userModel;  // 对数据库的操作都封装在model中
    OfflineMsgModel m_offlineMsgModel;
    FriendModel m_friendModel;
    GroupModel m_groupModel;

    Redis m_redis;
};