#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <functional>

typedef websocketpp::server<websocketpp::config::asio> server;

class websocket_server
{
public:
    websocket_server(); 

    void echo_handler(websocketpp::connection_hdl hdl, server::message_ptr msg);

    void run();
private:    
    server m_endpoint;
};
