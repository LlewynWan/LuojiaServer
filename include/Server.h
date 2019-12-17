#ifndef SERVER_H
#define SERVER_H

#include <MysqlQuery.h>

#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Server: private MysqlQuery
{
private:
    int serv_sock;
    struct sockaddr_in serv_addr;

    void login(std::string username, std::string password);
    void signup(std::string username, std::string password);
    void recommend();
    void search(std::string label);
    void add_commodity(std::string userID, std::string id, std::string type);
    void remove_commodity(std::string userID, std::string id, std::string type);
    void become_shopper(std::string username, std::string QQ, std::string wechat, std::string tel);
    void alter_commodity(std::string id);

public:
    Server(std::string addr = "127.0.0.1", int port = 1555);

    static const int MAX_BUFF_SIZE;
    static const int maxThreadCnt;
    static void* clntThread(void* args);
    void run();
};

#endif
