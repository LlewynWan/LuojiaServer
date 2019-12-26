#ifndef MYSQLQUERY_H
#define MYSQLQUERY_H

#include <string>
#include <mysql/mysql.h>

class MysqlQuery
{
protected:
    bool _isConnected;

public:
    MysqlQuery(std::string server = "localhost", std::string user = "root", std::string password = "password", std::string database = "Luojia");
    
    MYSQL* conn;
    bool isConnected();
};

#endif
