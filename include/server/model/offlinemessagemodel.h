#pragma once

#include "offlinemessage.hpp"

#include <string>
#include <vector>
using namespace std;

class OfflineMsgModel
{
public:
    // 存储用户离线消息
    bool insert(OfflineMessage& om);

    // 删除用户离线消息
    bool remove(int userid);

    // 查询用户离线消息
    vector<string> query(int userid);
};  