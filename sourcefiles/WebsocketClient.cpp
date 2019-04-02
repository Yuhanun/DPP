#include "sourcefiles/WebsocketClient.h"

WebsocketClient::WebsocketClient()
{
    client c;

    try {
        c.clear_access_channels(websocketpp::log::alevel::all);
        c.clear_error_channels(websocketpp::log::elevel::all);

        c.init_asio();

        c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

        std::string url = "wss://gateway.discord.gg/?v=6&encoding=json";

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(url, ec);
        c.connect(con);

        c.run();

        while (true){    

            c.reset();
    
            con = c.get_connection(url, ec);

            c.connect(con);

            c.run();
        }
        std::cout << "done" << std::endl;
    }

    catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

void WebsocketClient::on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << msg->get_payload() << std::endl;
}