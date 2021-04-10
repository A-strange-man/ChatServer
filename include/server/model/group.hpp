#pragma once

#include "groupuser.hpp"

#include <string>
#include <vector>

using namespace std;

class Group
{
public:
    Group(int i = -1, string name = "", string desc = "") : id(i), groupname(name), groupdesc(desc)
    { }

    void setId(int id) { this->id = id; }
    void setName(string name) { groupname = name; }
    void setDesc(string desc) { groupdesc = desc; }

    int getId() { return id; }
    string getName() { return groupname; }
    string getDesc() { return groupdesc; }
    vector<GroupUser> getUsers() { return users; }

private:
    int id;
    string groupname;
    string groupdesc;
    vector<GroupUser> users;
};