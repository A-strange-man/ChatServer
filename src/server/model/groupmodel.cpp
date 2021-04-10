#include "groupmodel.h"
#include "db.h"

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values ('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());
    
    MySql msql;
    if (msql.connect())
    {
        if (msql.update(sql))
        {
            group.setId(mysql_insert_id(msql.getConn()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(groupid, userid, grouprole) values (%d, %d, '%s')",
            userid, groupid, role.c_str());

    MySql msql;
    if (msql.connect())
    {
        msql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.goupdesc from AllGroup a inner join \
            GroupUser b on a.id=b.groupid where b.userid=%d", userid);
    
    vector<Group> groupVec;

    MySql msql;
    if (msql.connect())
    {
        MYSQL_RES* res = msql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);

        // 查询群组的用户信息
        for (Group& group : groupVec)
        {
            sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a \
                    inner join GroupUser b on b.userid=a.id where b.groupid=%d", group.getId());

            MYSQL_RES* res2 = msql.query(sql);
            if (res2 != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res2)) != nullptr)
                {
                    GroupUser groupUser;
                    groupUser.setId(atoi(row[0]));
                    groupUser.setName(row[1]);
                    groupUser.setState(row[2]);
                    groupUser.setRole(row[3]);
                    group.getUsers().push_back(groupUser);
                }
            }
            mysql_free_result(res2);
        }
    }

    return groupVec;
}

vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid!=%d", groupid, userid);

    vector<int> useridVec;
    
    MySql msql;
    if (msql.connect())
    {
        MYSQL_RES* res = msql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                useridVec.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return useridVec;
}