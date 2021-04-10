#include "offlinemessagemodel.h"
#include "db.h"

// 存储用户离线消息
bool OfflineMsgModel::insert(OfflineMessage& om)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage(userid,message) values (%d, '%s')", 
            om.getid(), om.getMsg().c_str());
    
    MySql msql;
    if (msql.connect())
    {
        if (msql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 删除用户离线消息
bool OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySql msql;
    if (msql.connect())
    {
        if (msql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 查询用户离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);

    vector<string> vec;
    MySql msql;
    if (msql.connect())
    {
        MYSQL_RES* res = msql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            
            mysql_free_result(res);
        }
    }
    return vec;
}