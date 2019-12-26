#include <websocket_server.h> 

websocket_server::websocket_server()
{
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

    m_endpoint.init_asio();

    m_endpoint.set_message_handler(std::bind(&websocket_server::echo_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void websocket_server::echo_handler(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    m_endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
}

void websocket_server::run()
{
    m_endpoint.listen(9002);
    m_endpoint.start_accept();
    m_endpoint.run();
}
