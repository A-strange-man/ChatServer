#pragma once

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
    LOGINOUT_MSG     = 10,       // 退出登陆
};