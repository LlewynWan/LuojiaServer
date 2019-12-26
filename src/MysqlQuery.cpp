#include <MysqlQuery.h>

#include <iostream>

MysqlQuery::MysqlQuery(std::string server, std::string user, std::string password, std::string database)
{
    conn = mysql_init(NULL);
    _isConnected = mysql_real_connect(conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0);
    
    if (!_isConnected)
        std::cerr << mysql_error(conn) << std::endl;
}

bool MysqlQuery::isConnected()
{
    return _isConnected;
}
