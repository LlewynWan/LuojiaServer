#include <Server.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <unistd.h>
#include <pthread.h>

const int Server::MAX_BUFF_SIZE = 65535;
const int Server::maxThreadCnt = 1000;

void Server::login(std::string username, std::string password)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT password FROM accounts WHERE sname = " << username << ";";
    std::string query = QueryBuffer.str();
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Login Aborted" << std::endl;
    }
    else {
	MYSQL_ROW row;
        MYSQL_RES* res = mysql_store_result(conn);
	if ((row = mysql_fetch_row(res)) == NULL)
	    std::cerr << "No Accounts Found" << std::endl;
	else {
	    std::string _password = row[0];
	    if (password.compare(_password) == 0)
	        std::cout << "You have successfully logged in." << std::endl;
	    else
		std::cerr << "Incorrect Password" << std::endl;
	}
    }
}

void Server::signup(std::string username, std::string password)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT * FROM accounts WHERE sname = \"" << username << "\";";
    std::string query = QueryBuffer.str();
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Signup Aborted" << std::endl;
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);

	if (mysql_fetch_row(res) != NULL)
	    std::cerr << "Username " << username << " Already Exists" << std::endl;
	else {
	    QueryBuffer.clear();
	    QueryBuffer.str("");
	    QueryBuffer << "SELECT MAX(ID) FROM accounts;";
	    query = QueryBuffer.str();
	    
	    if (mysql_query(conn, query.c_str()))
	    {
	        std::cerr << mysql_error(conn) << std::endl;
		std::cerr << "Signup Aborted" << std::endl;
	    }
	    else {
		int userID;
	        res = mysql_store_result(conn);
		row = mysql_fetch_row(res);
		if (row == NULL) userID = 10000;
		else userID = atoi(row[0])+1;

		QueryBuffer.clear();
		QueryBuffer.str("");
	        QueryBuffer << "INSERT INTO accounts VALUE (" << userID << ",\"" << username << "\",\"" << password << "\"," << 0 << ",\"\");";
		query = QueryBuffer.str();
		if (mysql_query(conn, query.c_str()))
		{
		    std::cerr << mysql_error(conn) << std::endl;
		    std::cerr << "Signup Aborted" << std::endl;
		}
		else {
		    std::cout << "You have successfully signed up." << std::endl;
		}
	    }
	}
    }
}

void Server::recommend()
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT * FROM commodity;";
    std::string query = QueryBuffer.str();
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Recommendation Aborted" << std::endl;
    }
    else {
	int cnt = 0;
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);
	
	srand(time(NULL));
	while ((row = mysql_fetch_row(res)) != NULL && cnt <= 100)
	{
	    if (rand() % 2) continue;

	    cnt++;
	    std::cout << "{goods: " << row[0] << ";";
	    std::cout << "label: " << row[1] << ";";
	    std::cout << "s_id: " << row[2] << ";";
	    std::cout << "price: " << row[3] << ";";
	    std::cout << "number: " << row[4] << "}";
	}
    }
}

void Server::search(std::string label)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT * FROM commodity WHERE label = " << label << ";";
    std::string query = QueryBuffer.str();
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Search Aborted" << std::endl;
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);
	while (row = mysql_fetch_row(res))
	{
	    std::cout << "{goods: " << row[0] << ";";
            std::cout << "label: " << row[1] << ";";
            std::cout << "s_id: " << row[2] << ";";
            std::cout << "price: " << row[3] << ";";
            std::cout << "number: " << row[4] << "}";
	}
    }
}

void Server::become_shopper(std::string username, std::string QQ, std::string wechat, std::string tel)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT seller FROM accounts WHERE sname = \"" << username << "\";";
    std::string query = QueryBuffer.str();
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Shopper Mode Enabling Aborted" << std::endl;
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);
	if ((row = mysql_fetch_row(res)) == NULL)
	    std::cerr << "No Accounts Found Under Username: " << username;
	else {
	    int isSeller = atoi(row[0]);
	    if (isSeller == 1)
	        std::cerr << "Shopper Mode Aleady Enabled" << std::endl;
	    else {
	        QueryBuffer.clear();
	        QueryBuffer.str("");
	        QueryBuffer << "UPDATE accounts SET seller = 1 WHERE username = " << username << ";";
		query = QueryBuffer.str();
		if (mysql_query(conn, query.c_str()))
		{
		    std::cerr << mysql_error(conn) << std::endl;
		    std::cerr << "Shopper Mode Enabling Aborted" << std::endl;
		}
		else {
		    QueryBuffer.clear();
		    QueryBuffer.str("");
		    QueryBuffer << "SELECT MAX(ID) FROM merchant;";
		    query = QueryBuffer.str();
		    if (mysql_query(conn, query.c_str()))
		    {
		        std::cerr << mysql_error(conn) << std::endl;
			std::cerr << "Shopper Mode Enabling Aborted" << std::endl;
		    }
		    else {
			int merchantID;
			res = mysql_store_result(conn);
			row = mysql_fetch_row(res);
			if (row == NULL) merchantID = 10000;
			else merchantID = atoi(row[0]);
			
			QueryBuffer.clear();
			QueryBuffer.str("");
			QueryBuffer << "INSERT INTO merchant VALUE (" << merchantID << "," << QQ << ",\"" << wechat << "\"," << tel << "\"\");";
			query = QueryBuffer.str();
			if (mysql_query(conn, query.c_str()))
			{
			    std::cerr << mysql_error(conn) << std::endl;
			    std::cerr << "Shopper Mode Enabling Aborted" << std::endl;
			}
			else
			    std::cout << "You have already become a shopper" << std::endl;
		    }
		}
	    }
        }
    }
}

void Server::add_commodity(std::string userID, std::string id, std::string type)
{
    std::stringstream QueryBuffer;
    if (type.compare("cart") == 0)
        QueryBuffer << "SELECT trolley FROM accounts ";
    else
	QueryBuffer << "SELECT shop FROM merchant ";
    QueryBuffer << "WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();

    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Adding Commodity Aborted" << std::endl;
    }
    else {
	std::string list;
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);
	
	row = mysql_fetch_row(res);
	if (row != NULL)
	    list.append(std::string(row[0])+",");
	list.append(id);

	QueryBuffer.clear();
	QueryBuffer.str("");
	if (type.compare("cart") == 0)
	    QueryBuffer << "UPDATE accounts SET trolley = ";
	else
	    QueryBuffer << "UPDATE merchant SET shop = ";
	QueryBuffer << "\"" << list << "\" WHERE ID = " << userID << ";";
	query = QueryBuffer.str();
	if (mysql_query(conn, query.c_str()))
	{
	    std::cerr << mysql_error(conn) << std::endl;
	    std::cerr << "Adding Commodity Aborted" << std::endl;
	}
	else
	    std::cout << "Commodity Added" << std::endl;
    }
}

void Server::remove_commodity(std::string userID, std::string id, std::string type)
{
    std::stringstream QueryBuffer;
    if (type.compare("cart") == 0)
        QueryBuffer << "SELECT trolley FROM accounts ";
    else
	QueryBuffer << "SELECT shop FROM merchant ";
    QueryBuffer << "WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();
    
    if (mysql_query(conn, query.c_str()))
    {
	std::cerr << mysql_error(conn) << std::endl;
	std::cerr << "Removing Commodity Aborted" << std::endl;
    }
    else {
	std::string list;
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(conn);
	
	row = mysql_fetch_row(res);
	if (row != NULL)
	    list.append(row[0]);
	int pos = -1;
	while (pos != std::string::npos)
	{
	    pos = list.find(id, pos+1);
	    if (pos+id.length()==list.length() || list[pos+id.length()]==',')
	        break;
	}
	if (pos == std::string::npos)
	    std::cerr << "Commodity Not Found" << std::endl;
	else {
	    if (pos == 0)
	    {
	        if (list.find(",") == std::string::npos)
		    list.replace(pos, id.length(), "");
		else
		    list.replace(pos, id.length()+1, "");
	    }
	    else
		list.replace(pos-1, id.length()+1, "");
	    
	    QueryBuffer.clear();
	    QueryBuffer.str("");
	    if (type.compare("cart") == 0)
	        QueryBuffer << "UPDATE accounts SET trolley = ";
	    else
		QueryBuffer << "UPDATE merchant SET shop = ";
	    QueryBuffer << "\"" << list << "\" WHERE ID = " << userID << ";";
	    query = QueryBuffer.str();

	    if (mysql_query(conn, query.c_str()))
	    {
	        std::cerr << mysql_error(conn) << std::endl;
		std::cerr << "Removing Commodity Aborted" << std::endl;
	    }
	    else
		std::cout << "Commodity Removed" << std::endl;
	}
    }
}

Server::Server(std::string addr, int port)
{
    MysqlQuery();

    serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(addr.c_str());
    serv_addr.sin_port = htons(port);
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(serv_sock, 1000);
}


void* Server::clntThread(void *args)
{
    std::cout << "Connected!" << std::endl;
    int clnt_sock = *(int*)args;
    std::string message("server");
    write(clnt_sock, message.c_str(), message.size());

    char buffer[MAX_BUFF_SIZE];
    read(clnt_sock, buffer, sizeof(buffer)-1);
    std::cout << buffer << std::endl;
}

void Server::run()
{
    int top = 0;
    pthread_t tid[maxThreadCnt];
    for (;;)
    {
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);

        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if (pthread_create(&tid[top++], NULL, Server::clntThread, (void*)&clnt_sock) != 0)
	    std::cerr << "Failed to Create Thread" << std::endl;

	if (top >= maxThreadCnt)
	{
	    top = 0;
	    while (top < maxThreadCnt)
	        pthread_join(tid[top++], NULL);
	    top = 0;
	}
    }
}
