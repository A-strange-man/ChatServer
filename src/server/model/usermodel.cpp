#include "usermodel.h"
#include "db.h"

#include <muduo/base/Logging.h>
using namespace muduo;

bool UserModel::insert(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values ('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    
    MySql msql;
    if (msql.connect())
    {
        if (msql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(msql.getConn()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id=%d", id);

    MySql msql;
    if (msql.connect())
    {
        MYSQL_RES* res = msql.query(sql);
        User user;
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);   // 拿到查询结果，查询结果是数据表的一行信息
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);
                return user;
            }
            LOG_INFO << "mysql_fetch_row() failed!";
            return user;
        }
        return user;
    }
}

bool UserModel::updateState(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state='%s' where id=%d", user.getState().c_str(), user.getId());

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

bool UserModel::resetState()
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state='offline' where state='online'");

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