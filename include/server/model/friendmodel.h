#pragma once

#include "user.hpp"
#include <vector>
using namespace std;

// 维护好友信息的操作接口
class FriendModel
{
public:
    void insert(int userid, int friendid);      // 添加好友

    // void remove(int userid, int friendid);      // 删除好友

    vector<User> querey(int userid);            // 返回用户好友列表
};