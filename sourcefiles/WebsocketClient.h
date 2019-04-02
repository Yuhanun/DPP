#ifndef WebsocketClient_H
#define WebsocketClient_H

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class WebsocketClient
{
    public:
        WebsocketClient();
        void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg);
};
#endif