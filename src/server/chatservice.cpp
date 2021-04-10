#include "chatservice.h"
#include "public.h"

#include <muduo/base/Logging.h>
#include <string>
#include <vector>

using namespace muduo;
using namespace std;

ChatService* ChatService::getInstance()
{
    static ChatService service;
    return &service;
}

// 在map表中注册服务   <服务号 , 对应的处理函数>
ChatService::ChatService()
{
    m_MsgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addgroup, this, _1, _2, _3)});
    m_MsgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (m_redis.connect())
    {
        // 设置上报消息的回调
        m_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 登陆   id + password
void ChatService::login(const TcpConnectionPtr & conn, string & js, Timestamp time)
{
    json loginInfo = json::parse(js);
    int id = loginInfo["id"].get<int>();
    string pwd = loginInfo["password"];

    User user = m_userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登陆，不能重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已登陆!";
            conn->send(response.dump());
        }
        else
        {
            // 登陆成功
            user.setState("online");
            if (m_userModel.updateState(user))
            {
                // 往在线列表里添加一条记录
                {
                    lock_guard<mutex> lg(m_connMutex);
                    m_userConnMap.insert({id, conn});
                }

                // 向redis订阅channel（即id）
                m_redis.subscribe(user.getId());

                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 0;
                response["id"] = user.getId();
                response["name"] = user.getName();

                // 检查用户是否有离线消息，有则一并返回给用户
                vector<string> vec = m_offlineMsgModel.query(id);
                if (!vec.empty())
                {
                    response["offlinemsg"] = vec;
                    m_offlineMsgModel.remove(id);
                }

                //  查询用户的好友信息并返回
                vector<User> userVec = m_friendModel.querey(id);
                if (!userVec.empty())
                {
                    vector<string> vec2;
                    for (User& user : userVec)
                    {
                        json temp;
                        temp["id"] = user.getId();
                        temp["name"] = user.getName();
                        temp["state"] = user.getState();
                        vec2.push_back(temp.dump());
                    }
                    response["friends"] = vec2;
                }

                // 返回数据给客户端
                conn->send(response.dump());
            }

        }
    }
    else
    {
        // 登陆失败   用户不存在 或者 账号/密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误!";
        // 返回数据给客户端
        conn->send(response.dump());
    }
}

// 注册   name + password
void ChatService::reg(const TcpConnectionPtr & conn, string & js, Timestamp time)
{
    json reginfo = json::parse(js);
    string name = reginfo["name"];
    string pwd = reginfo["password"];
    
    User user;
    user.setName(name);
    user.setPwd(pwd);
    
    bool regState = m_userModel.insert(user);   //新用户注册
    if (regState)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        // 返回数据给客户端
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        // 返回数据给客户端
        conn->send(response.dump());
    }
}

// 用户退出
void ChatService::loginOut(const TcpConnectionPtr & conn, string & js, Timestamp time) 
{
    json loginOutInfo = json::parse(js);
    int userId = loginOutInfo["id"].get<int>();
    
    {
        lock_guard<mutex> lock(m_connMutex);
        
        auto it = m_userConnMap.find(userId);
        if (it != m_userConnMap.end()) 
        {
            m_userConnMap.erase(it);
        }
    }

    // 用户退出，向redis-server取消订阅
    m_redis.unsubscribe(userId);

    User user;
    user.setId(userId);
    user.setState("offline");
    // 更新数据表对应的状态信息
    m_userModel.updateState(user);
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr & conn, string & js, Timestamp time)
{
    json oneChatMsg = json::parse(js);
    int toid = oneChatMsg["toid"].get<int>();
    
    {
        lock_guard<mutex> lg(m_connMutex);
        auto it = m_userConnMap.find(toid);
        if (it != m_userConnMap.end())
        {
            // 对方在线,转发消息
            it->second->send(js);
            return;
        }    
    }

    // 查询用户是否在线（因为用户可能连接的其它服务器）
    User user = m_userModel.query(toid);
    if (user.getState() == "online")
    {
        // 用户在线，但连接的另一台服务器
        m_redis.publish(toid, js);
        return;
    }

    // 不在线,存储离线消息
    OfflineMessage offlinemsg(toid, js);
    m_offlineMsgModel.insert(offlinemsg);
}

// 添加好友     msgid + id + friendid
void ChatService::addFriend(const TcpConnectionPtr & conn, string & js, Timestamp time)
{
    json addFriendMsg = json::parse(js);
    int userid = addFriendMsg["id"].get<int>();
    int friendid = addFriendMsg["friendid"].get<int>();

    m_friendModel.insert(userid, friendid);
}

// 创建群聊
void ChatService::createGroup(const TcpConnectionPtr &conn, string &js, Timestamp time)
{
    json createGroupMsg = json::parse(js);
    int userid = createGroupMsg["id"].get<int>();
    string name = createGroupMsg["groupname"];
    string desc = createGroupMsg["groupdesc"];

    Group group;
    group.setName(name);
    group.setDesc(desc);

    if (m_groupModel.createGroup(group))
    {
        m_groupModel.addGroup(group.getId(), userid, "creator");
    }
}

// 添加群聊
void ChatService::addgroup(const TcpConnectionPtr &conn, string &js, Timestamp time)
{
    json addGroupMsg = json::parse(js);
    int userid = addGroupMsg["id"].get<int>();
    int groupid = addGroupMsg["groupid"];  

    m_groupModel.addGroup(groupid, userid, "normal");
}

// 群发消息
void ChatService::groupChat(const TcpConnectionPtr &conn, string &js, Timestamp time)
{
    json groupChatMsg = json::parse(js);
    int userid = groupChatMsg["id"];
    int groupid = groupChatMsg["groupid"];

    vector<int> groupMember;
    groupMember = m_groupModel.queryGroupUsers(userid, groupid);
    
    lock_guard<mutex> lg(m_connMutex);
    for (int& toid : groupMember)
    {
        auto it = m_userConnMap.find(toid);
        if (it != m_userConnMap.end())
        {
            it->second->send(js);
        }
        else
        {
            // 查询 toid是否在线
            User user = m_userModel.query(toid);
            if (user.getState() == "online")
            {
                m_redis.publish(toid, js);
            }
            else
            {
                OfflineMessage offlineMsg(toid, js);
                m_offlineMsgModel.insert(offlineMsg);
            }
        }
    }
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        lock_guard<mutex> lg(m_connMutex);
        for (auto it = m_userConnMap.begin(); it != m_userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                m_userConnMap.erase(it);
                break;
            }
        }
    }

    // 取消订阅
    m_redis.unsubscribe(user.getId());

    user.setState("offline");
    // 更新数据表对应的状态信息
    m_userModel.updateState(user);
}

// 服务器退出,重置用户登陆状态
void ChatService::reset()
{
    m_userConnMap.clear();
    m_userModel.resetState();
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = m_MsgHandlerMap.find(msgid);
    if (it == m_MsgHandlerMap.end())
    {
        // 返回一个默认处理器（仅打印错误消息）
        return [=](const TcpConnectionPtr & conn, string & js, Timestamp time){
            LOG_ERROR << "msgid:" << msgid << " can't find msgHandler";
        };
    }
    else
    {
        return m_MsgHandlerMap[msgid];
    }
}

// 处理redis-server上报的消息
void ChatService::handleRedisSubscribeMessage(int channel, string msg)
{
    // 在当前主机查找用户
    lock_guard<mutex> lg(m_connMutex);
    auto it = m_userConnMap.find(channel);
    if (it != m_userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 用户不在线，存储离线消息
    OfflineMessage offlineMsg(channel, msg);
    m_offlineMsgModel.insert(offlineMsg);
}