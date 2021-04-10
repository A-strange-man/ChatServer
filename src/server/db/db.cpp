#include "db.h"

#include <muduo/base/Logging.h>
using namespace muduo;

MySql::MySql()
{
    m_sqlConn = mysql_init(nullptr);
}

MySql::~MySql()
{
    if (m_sqlConn != nullptr)
    {
        mysql_close(m_sqlConn);
    }
}

bool MySql::connect()
{
    MYSQL *p = mysql_real_connect(m_sqlConn, server.c_str(), user.c_str(), password.c_str(),
                                  dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(m_sqlConn, "set names gbk");
        LOG_INFO << "connect mysql success";
    }
    else
    {
        LOG_INFO << "connect mysql fail";
    }
    return p;
}

bool MySql::update(std::string sql)
{
    if (mysql_query(m_sqlConn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << __LINE__ << ": " << sql << " 更新失败！";
        return false;
    }
    return true;
}

MYSQL_RES* MySql::query(std::string sql)
{
    if (mysql_query(m_sqlConn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << __LINE__ << ": " << sql << " 查询失败！";
        return nullptr;
    }
    return mysql_use_result(m_sqlConn);
}

MYSQL* MySql::getConn()
{
    return m_sqlConn;
}