#include <server.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <pthread.h>

const int Server::MAX_BUFF_SIZE = 65535;
const int Server::maxThreadCnt = 1000;
const int Server::IMG_BUFF_SIZE = 1024;

const int Server::IMG_MAX_BUFF = 2500005;

const std::string Server::image_file_path = "image/";

const MysqlQuery* Server::MysqlLogger = new MysqlQuery();

const std::string errmsg("?");
const std::string tab("NULL");

void Server::login(std::string userID, std::string password, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT password, sname FROM accounts WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Login Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
    }
    else {
	MYSQL_ROW row;
        MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
	row = mysql_fetch_row(res);
        if (res == NULL || mysql_num_rows(res) == 0)
	{
	    std::cerr << "No Accounts Found" << std::endl;
	    write(clnt_sock, errmsg.c_str(), errmsg.size());
	}
	else {
	    std::string _password = row[0];
	    if (password.compare(_password) == 0)
	    {
		std::string username = row[1];
	        std::cout << "User has successfully logged in." << std::endl;
	        write(clnt_sock, username.c_str(), username.size());
	    }
	    else
	    {
		std::cerr << "Incorrect Password" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
	    }
	}
    }
}

void Server::signup_common(std::string username, std::string password, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT MAX(ID) FROM accounts;";
    std::string query = QueryBuffer.str();
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Signup Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
  	
	int userID;
	row = mysql_fetch_row(res);
	if (row == NULL || res == NULL || row[0] == NULL) userID = 10000;
	else userID = atoi(row[0])+1;

	QueryBuffer.clear();
	QueryBuffer.str("");
	QueryBuffer << "INSERT INTO accounts VALUE (" << userID << ",\"" << username << "\",\"" << password << "\"," << 0 << ",\"\");";
	query = QueryBuffer.str();
	
	if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	{
	    std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	    std::cerr << "Signup Aborted" << std::endl;
            write(clnt_sock, errmsg.c_str(), errmsg.size());
	}
	else {
	    std::cout << "User has successfully signed up." << std::endl;
	    std::string uidMsg = std::to_string(userID);
	    write(clnt_sock, uidMsg.c_str(), uidMsg.size());
	}
    }
}

void Server::get_shopper_info(std::string userID, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT name,tel,loc,major,grade FROM merchant WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Shopper Info Query Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);

	row = mysql_fetch_row(res);
	if (row == NULL)
	{
	    std::cerr << "No Shopper Accounts Found" << std::endl;
	    write(clnt_sock, errmsg.c_str(), errmsg.size());
	} else {
	    std::string infoBuffer(row[0]);
	    infoBuffer.append(";"+std::string(row[1]));
	    infoBuffer.append(";"+std::string(row[2]));
	    infoBuffer.append(";"+std::string(row[3]));
	    infoBuffer.append(";"+std::string(row[4]));
	    write(clnt_sock, infoBuffer.c_str(), infoBuffer.size());
	    std::cout << "Shopper Info Successfully Sent." << std::endl;
	}
    }
}

void Server::recommend(int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT * FROM commodity;";
    std::string query = QueryBuffer.str();
    char tmp[IMG_BUFF_SIZE];
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Recommendation Aborted" << std::endl;
    }
    else {
	MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
	
	int cnt = std::min(5, (int)mysql_num_rows(res));
        read(clnt_sock, tmp, sizeof(tmp)-1);
	write(clnt_sock, std::to_string(cnt).c_str(), std::to_string(cnt).size());
	
	while ((row = mysql_fetch_row(res)) != NULL && cnt <= 100)
	{
	    cnt--;
	    
	    std::string name(row[3]);
	    std::string price(row[2]);
            std::string number(row[4]);
            std::string label(row[0]);
	    std::string s_id(row[1]);
	    std::string infoBuffer;
	    infoBuffer.append(name+";");
	    infoBuffer.append(price+";");
	    infoBuffer.append(number+";");
	    infoBuffer.append(label);

	    char image_buff[IMG_BUFF_SIZE];
	    read(clnt_sock, image_buff, sizeof(image_buff)-1);
	    write(clnt_sock, infoBuffer.c_str(), infoBuffer.size());

	    std::string file_path = Server::image_file_path + s_id;
	    FILE* file = fopen(file_path.c_str(), "rb");
	    fseek(file, 0, SEEK_END);
	    int imgLen = ftell(file);
	    fseek(file, 0, SEEK_SET);
	    std::string imgLenStr = std::to_string(imgLen);
	    read(clnt_sock, image_buff, sizeof(image_buff)-1);
	    write(clnt_sock, imgLenStr.c_str(), imgLenStr.size());

	    int numPack = imgLen / IMG_BUFF_SIZE;
            for (int j = 0; j < numPack; j++)
	    {
	        read(clnt_sock, image_buff, sizeof(image_buff)-1);
	        memset(image_buff, 0, sizeof(image_buff));
	        fread(image_buff, 1, IMG_BUFF_SIZE, file);
	        write(clnt_sock, image_buff, IMG_BUFF_SIZE);
	    }
	    if (imgLen % IMG_BUFF_SIZE)
	    {
	        read(clnt_sock, image_buff, IMG_BUFF_SIZE);
	        memset(image_buff, 0, sizeof(image_buff));
	        fread(image_buff, 1, imgLen % IMG_BUFF_SIZE, file);
	        write(clnt_sock, image_buff, imgLen % IMG_BUFF_SIZE);
	    }
	    fclose(file);

	    if (cnt == 0)
		 break;
	}
        std::cout << "Recommened Items Info Successfully Sent." << std::endl;
    }
}

void Server::search(std::string label)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT * FROM commodity WHERE label = " << label << ";";
    std::string query = QueryBuffer.str();
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Search Aborted" << std::endl;
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
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

void Server::register_shopper(std::string userID, std::string name, std::string loc, std::string tel, std::string major, std::string grade, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT seller FROM accounts WHERE ID = \"" << userID << "\";";
    std::string query = QueryBuffer.str();
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Shopper Register Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
    }
    else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
	row = mysql_fetch_row(res);

	if (row == NULL)
	{
	    std::cerr << "No Accounts Found" << std::endl;
	    write(clnt_sock, errmsg.c_str(), errmsg.size());
	}
	else {
	    int isSeller = atoi(row[0]);
	    if (isSeller == 1)
	    {
	        std::cerr << "Already Registered Before" << std::endl;
	        write(clnt_sock, errmsg.c_str(), errmsg.size());
	    }
	    else {
	        QueryBuffer.clear();
	        QueryBuffer.str("");
	        QueryBuffer << "UPDATE accounts SET seller = 1 WHERE sname = \"" << userID << "\";";
		query = QueryBuffer.str();
		if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
		{
		    std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
		    std::cerr << "Shopper Register Aborted" << std::endl;
		    write(clnt_sock, errmsg.c_str(), errmsg.size());
		}
		else {
		    QueryBuffer.clear();
		    QueryBuffer.str("");
		    QueryBuffer << "INSERT INTO merchant VALUE (" << userID << ",\"" << name << "\",\"" << loc << "\"," << tel << ",\"\",\"" << major << "\",\"" << grade << "\");";
		    query = QueryBuffer.str();
		    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
		    {
		        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
			std::cerr << "Shopper Register Aborted" << std::endl;
			write(clnt_sock, errmsg.c_str(), errmsg.size());
		    }
		    else {	
			std::cout << "User has successfully registered as a shopper." << std::endl;
			write(clnt_sock, std::string("true").c_str(), std::string("true").size());
		    }
		}
	    }
        }
    }
}

void Server::add_commodity(std::string userID, std::string name, std::string price, std::string number, std::string label, char* image, int imgLen, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT MAX(s_id) FROM commodity;";
    std::string query = QueryBuffer.str();

    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
	std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Adding Commodity Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
    } else {
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);

	row = mysql_fetch_row(res);
	if (row == NULL)
	{
	    std::cerr << "Adding Commodity Aborted" << std::endl;
	    write(clnt_sock, errmsg.c_str(), errmsg.size());
	} else {
	    int s_id;
	    if (row[0] == NULL) s_id = 0;
	    else s_id = atoi(row[0])+1;
	    
	    QueryBuffer.clear();
	    QueryBuffer.str("");
	    QueryBuffer << "SELECT shop FROM merchant WHERE ID = " << userID << ";";
	    query = QueryBuffer.str();
	    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	    {
	        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
		std::cerr << "Adding Commodity Aborted" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		return;
	    }
	    res = mysql_store_result(Server::MysqlLogger->conn);
	    row = mysql_fetch_row(res);

	    std::string list(row[0]);
	    if (row[0] != NULL && !list.empty())
		list.append(std::string(","));
	    list.append(std::to_string(s_id));

	    QueryBuffer.clear();
	    QueryBuffer.str("");
	    QueryBuffer << "INSERT INTO commodity VALUE (" << label << "," << s_id << "," << price << ",\"" << name << "\"," << number << ");";
	    query = QueryBuffer.str();
	    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	    {
		std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
		std::cerr << "Adding Commodity Aborted" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		return;
	    } 

	    QueryBuffer.clear();
	    QueryBuffer.str("");
	    QueryBuffer << "UPDATE merchant SET shop = \"" << list << "\" WHERE ID = " << userID << ";";
	    query = QueryBuffer.str();
	    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	    {
                std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
		std::cerr << "Adding Commodity Aborted" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
	    } else {
	        std::cout << "User has successfully added commodity." << std::endl;

		std::string image_path = Server::image_file_path + std::to_string(s_id);
		FILE* image_file = fopen(image_path.c_str(), "wb");
		fwrite(image, 1, imgLen, image_file);
		fclose(image_file);

		write(clnt_sock, std::string("true").c_str(), std::string("true").size());
	    }
	}
    }
}

void Server::remove_commodity(std::string userID, std::string id, std::string type, int clnt_sock)
{
    std::stringstream QueryBuffer;
    if (type.compare("cart") == 0)
        QueryBuffer << "SELECT trolley FROM accounts ";
    else
	QueryBuffer << "SELECT shop FROM merchant ";
    QueryBuffer << "WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();
    
    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
	std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Removing Commodity Aborted" << std::endl;
    }
    else {
	std::string list;
        MYSQL_ROW row;
	MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
	
	row = mysql_fetch_row(res);
        if (res != NULL && mysql_num_rows(res) != 0)
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

	    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	    {
	        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
		std::cerr << "Removing Commodity Aborted" << std::endl;
	    }
	    else
		std::cout << "Commodity Removed" << std::endl;
	}
    }
}

void Server::get_shopper_items(std::string userID, int clnt_sock)
{
    std::stringstream QueryBuffer;
    QueryBuffer << "SELECT shop FROM merchant WHERE ID = " << userID << ";";
    std::string query = QueryBuffer.str();

    if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
    {
        std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
	std::cerr << "Querying Shopper Items Aborted" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
	return;
    }

    MYSQL_ROW row;
    MYSQL_RES* res = mysql_store_result(Server::MysqlLogger->conn);
    row = mysql_fetch_row(res);
    
    if (row == NULL)
    {
	std::cerr << "No Shopper Accounts Found" << std::endl;
	write(clnt_sock, errmsg.c_str(), errmsg.size());
	return;
    }
    std::string list(row[0]);

    int pos = list.find(",");
    std::vector<int> items_id;
    if (pos == std::string::npos)
    {
	if (!list.empty())
            items_id.push_back(std::stoi(list));
    }
    else {
        items_id.push_back(std::stoi(list.substr(0,pos)));
        for (;;)
        {
            int next = list.find(",",pos+1);
	    if (next == std::string::npos)
	    {
	        items_id.push_back(std::stoi(list.substr(pos+1)));
	        break;
	    }
	    else
	        items_id.push_back(std::stoi(list.substr(pos+1,next-pos-1)));
	
	    pos = next;
        }
    }
    char tmp[IMG_BUFF_SIZE];
    std::string numItems = std::to_string(items_id.size());
    write(clnt_sock, numItems.c_str(), numItems.size());
    
    for (unsigned int i = 0; i < items_id.size(); i++)
    {
	read(clnt_sock, tmp, IMG_BUFF_SIZE);

        QueryBuffer.clear();
	QueryBuffer.str("");
	QueryBuffer << "SELECT * FROM commodity WHERE s_id = " << std::to_string(items_id[i]) << ";";
	query = QueryBuffer.str();

	if (mysql_query(Server::MysqlLogger->conn, query.c_str()))
	{
	    std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
            std::cerr << "Querying Shopper Items Aborted" << std::endl;
            write(clnt_sock, errmsg.c_str(), errmsg.size());
            return;
	}

	res = mysql_store_result(Server::MysqlLogger->conn);
	row = mysql_fetch_row(res);

	if (row == NULL)
	{
            std::cerr << mysql_error(Server::MysqlLogger->conn) << std::endl;
            std::cerr << "Querying Shopper Items Aborted" << std::endl;
            write(clnt_sock, errmsg.c_str(), errmsg.size());
            return;

	}

	std::string name(row[3]);
	std::string price(row[2]);
        std::string number(row[4]);
        std::string label(row[0]);	
	std::string infoBuffer;
	infoBuffer.append(name+";");
	infoBuffer.append(price+";");
	infoBuffer.append(number+";");
	infoBuffer.append(label);

	char image_buff[IMG_BUFF_SIZE];
	write(clnt_sock, infoBuffer.c_str(), infoBuffer.size());
	read(clnt_sock, image_buff, sizeof(image_buff)-1);

	std::string file_path = Server::image_file_path + std::to_string(items_id[i]);
	FILE* file = fopen(file_path.c_str(), "rb");
	fseek(file, 0, SEEK_END);
	int imgLen = ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string imgLenStr = std::to_string(imgLen);
	write(clnt_sock, imgLenStr.c_str(), imgLenStr.size());

	int numPack = imgLen / IMG_BUFF_SIZE;
        for (int j = 0; j < numPack; j++)
	{
	    read(clnt_sock, image_buff, sizeof(image_buff)-1);
	    memset(image_buff, 0, sizeof(image_buff));
	    fread(image_buff, 1, IMG_BUFF_SIZE, file);
	    write(clnt_sock, image_buff, IMG_BUFF_SIZE);
	}
	if (imgLen % IMG_BUFF_SIZE)
	{
	    read(clnt_sock, image_buff, IMG_BUFF_SIZE);
	    memset(image_buff, 0, sizeof(image_buff));
	    fread(image_buff, 1, imgLen % IMG_BUFF_SIZE, file);
	    write(clnt_sock, image_buff, imgLen % IMG_BUFF_SIZE);
	}
	fclose(file);
    }

    std::cout << "Shopper Items Info Successfully Sent." << std::endl;
}

Server::Server(std::string addr, int port)
{
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

    struct pollfd pfd;
    pfd.fd = clnt_sock;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
    pfd.revents = 0;
    while (pfd.revents == 0)
    {
	char buffer[MAX_BUFF_SIZE];
	memset(buffer, 0, sizeof(buffer));
        read(clnt_sock, buffer, sizeof(buffer)-1);
	write(clnt_sock, tab.c_str(), tab.size());
	std::string BuffStr(buffer);
	
	std::cout << BuffStr << std::endl;
	
	if (BuffStr.compare("verify password") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string userID(buffer);
	    write(clnt_sock, tab.c_str(), tab.size());
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string password(buffer); 
            login(userID, password, clnt_sock);
	}

	if (BuffStr.compare("register common user") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string username(buffer);
	    write(clnt_sock, tab.c_str(), tab.size());
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string password(buffer);
	    signup_common(username, password, clnt_sock);
	}

	if (BuffStr.compare("get seller info") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string userID(buffer);
	    get_shopper_info(userID, clnt_sock);
	}

	if (BuffStr.compare("register seller") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string userID(buffer);
	    write(clnt_sock, tab.c_str(), tab.size());
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string Info(buffer);

	    int pos = Info.find(";");
	    if (pos == std::string::npos)
	    {
	        std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string name = Info.substr(0, pos);
	    int next = Info.find(";", pos+1);
            if (next == std::string::npos)
	    {
	        std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string tel = Info.substr(pos+1, next-pos-1);
	    pos = next;
	    next = Info.find(";", pos+1);
            if (next == std::string::npos)
	    {
	        std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string loc = Info.substr(pos+1, next-pos-1);
	    pos = next;
	    next = Info.find(";", pos+1);
            if (pos == std::string::npos)
            {
                std::cerr << "Invalid Info String Received" << std::endl;
                write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
            }
	    std::string major = Info.substr(pos+1, next-pos-1);
	    std::string grade = Info.substr(next+1);

	    register_shopper(userID,name,tel,loc,major,grade,clnt_sock);
	}

	if (BuffStr.compare("seller add item") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string userID(buffer);
	    write(clnt_sock, tab.c_str(), tab.size());
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string Info(buffer);

	    int pos = Info.find(";");
	    if (pos == std::string::npos)
	    {
	        std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string name = Info.substr(0, pos);
	    int next = Info.find(";", pos+1);
	    if (next == std::string::npos)
	    {
		std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string price = Info.substr(pos+1, next-pos-1);
	    pos = next;
	    next = Info.find(";", pos+1);
	    if (next == std::string::npos)
	    {
		std::cerr << "Invalid Info String Received" << std::endl;
		write(clnt_sock, errmsg.c_str(), errmsg.size());
		continue;
	    }
	    std::string number = Info.substr(pos+1, next-pos-1);
	    std::string label = Info.substr(next+1);

	    write(clnt_sock, tab.c_str(), tab.size());
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    int imgLen = atoi(buffer);
	    int numPack = imgLen / IMG_BUFF_SIZE;

	    char image[IMG_MAX_BUFF];
	    memset(image, 0, sizeof(image));
	    for (int i = 0; i < numPack; i++)
	    {
	        write(clnt_sock, tab.c_str(), tab.size());
                read(clnt_sock,	image + IMG_BUFF_SIZE * i, IMG_BUFF_SIZE);
	    }
	    if (imgLen % IMG_BUFF_SIZE)
	    {
		write(clnt_sock, tab.c_str(), tab.size());
		read(clnt_sock, image + IMG_BUFF_SIZE * numPack, imgLen % IMG_BUFF_SIZE);
	    }

	    add_commodity(userID, name, price, number, label, image, imgLen, clnt_sock);
	}

	if (BuffStr.compare("get seller item") == 0)
	{
	    memset(buffer, 0, sizeof(buffer));
	    read(clnt_sock, buffer, sizeof(buffer)-1);
	    std::string userID(buffer);
	    get_shopper_items(userID, clnt_sock);
	}

	if (BuffStr.compare("get recommends") == 0)
	    recommend(clnt_sock);

	if (BuffStr.empty())
	    break;
    }
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
