#pragma once

#include "mysql_init.h"

#include <mysql/mysql.h>

class MySql
{
public:
    MySql();
    ~MySql();

    bool connect();
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);

    MYSQL* getConn();
private:
    MYSQL* m_sqlConn;
};

