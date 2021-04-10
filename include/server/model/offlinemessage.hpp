#pragma once

#include <string>
using namespace std;

class OfflineMessage
{
public:
    OfflineMessage(int id, string m) : userid(id), msg(m)
    { }
    
    void setid(int id) { userid = id; }
    void setmsg(string m) { msg = m; }
    
    int getid() { return userid; }
    string getMsg() { return msg; }  

private:
    int userid;
    string msg;
};