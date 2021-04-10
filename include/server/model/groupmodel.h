#pragma once

#include "group.hpp"

#include <string>
#include <vector>
using namespace std;

class GroupModel
{
public:
    bool createGroup(Group& group);     //创建群组

    void addGroup(int userid, int groupid, string role);    // 加入群组

    vector<Group> queryGroups(int userid);      //查询群组信息

    vector<int> queryGroupUsers(int userid, int groupid);   //查询指定群组的用户列表,用于给其它成员群发信息
};