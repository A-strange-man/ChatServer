#pragma once

#include "user.hpp"

#include <string>
using namespace std;

// 群组中的用户
class GroupUser : public User 
{
public:
    void setRole(string role) { this->role = role; }
    
    string getRole() { return this->role; } 

private:
    string role;
};