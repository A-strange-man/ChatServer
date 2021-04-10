#include "friendmodel.h"
#include "db.h"

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend(userid, friendid) values (%d, %d)", userid, friendid);

    MySql msql;
    if (msql.connect())
    {
        msql.update(sql);
    }
}

vector<User> FriendModel::querey(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid=a.id where b.userid=%d", userid);
    
    vector<User> vec;
    MySql msql;
    if (msql.connect())
    {
        MYSQL_RES* res = msql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
        }
        mysql_free_result(res);
    }
    return vec;
}