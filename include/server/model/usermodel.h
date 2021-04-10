#pragma once

#include "user.hpp"

// user表的数据操作类
class UserModel
{
public:
    bool insert(User& user);        // 用户注册
    User query(int id);             // 用户登陆，根据账号id查询数据库
    bool updateState(User& user);   // 更新用户状态
    bool resetState();              // 重置用户的状态信息
};