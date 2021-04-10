#pragma once

#include <string>
using namespace std;

class User
{
public:
    User(int i = -1, string n = "", string p = "", string s = "offline")
        : id(i), name(n), password(p), state(s)
    { }

    void setId(int id) { this->id = id; };
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return id; }
    string getName() { return name; }
    string getPwd() { return password; }
    string getState() { return state; }

private:
    int id;
    string name;
    string password;
    string state;
};