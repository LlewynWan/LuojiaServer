#ifndef SERVER_H
#define SERVER_H

#include <MysqlQuery.h>
#include <websocket_server.h>

#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Server
{
private:
    int serv_sock;
    struct sockaddr_in serv_addr;

    static const std::string image_file_path;

    static void login(std::string username, std::string password, int clnt_sock);
    static void signup_common(std::string username, std::string password, int clnt_sock);
    static void recommend(int clnt_sock);
    static void search(std::string label);
    static void alter_commodity(std::string id);
    static void get_shopper_info(std::string userID, int clnt_sock);
    static void register_shopper(std::string userID, std::string name, std::string tel, std::string loc, std::string major, std::string grade, int clnt_sock);
    static void add_commodity(std::string userID, std::string name, std::string price, std::string number, std::string label, char* image, int imgLen, int clnt_sock);
    static void remove_commodity(std::string userID, std::string id, std::string type, int clnt_sock);
    static void get_shopper_items(std::string userID, int clnt_sock);

    static const MysqlQuery* MysqlLogger;
    websocket_server WebSocket;

public:
    Server(std::string addr = "192.168.43.175", int port = 12345);

    static const int MAX_BUFF_SIZE;
    static const int IMG_BUFF_SIZE;
    static const int IMG_MAX_BUFF;
    static const int maxThreadCnt;
    static void* clntThread(void* args);
    void run();
};

#endif
