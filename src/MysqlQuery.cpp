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

/*
std::map<string, string>* MysqlQuery::select(vector<string> SELECTION, string TNAME, string constraint = "")
{
    stringstream QueryBuffer;
    QueryBuffer << "select ";
    if (SELECTION.size() == 1 && SELECTION[0].compare("*") == 0)
        QueryBuffer "* from " << TNAME;
    else {
        for (vector<string>::iterator iter = SELECTION.begin(); iter != SELECTION.end(); iter++)
	    QueryBuffer << *iter << " ";
	QueryBuffer << "from "<< TNAME;
    }

    QueryBuffer << constraint << ";";

    if (mysql_query(conn, QueryBuffer.str().c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	return NULL;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    std::map<string, string>* result = new std::map<string, string>;
}*/
