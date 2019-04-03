#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <atomic>

#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace nlohmann;

int packet_counter = 0;
json hello_packet;
std::string tkn;
client::connection_ptr con;

std::string get_identify_packet() {
    json obj = {
        { "op", 2 },
        {
            "d",
            {
                { "token", tkn },
                { "properties",
                    {
                        { "$os", "Linux" },
                        { "$browser", "DiscordPP" },
                        { "$device", "DiscordPP" }
                    }
                },
                { "compress", false },
                { "large_threshold", 250 }
            }
        }
    };
    return obj.dump();
}

void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    json j = json::parse(msg->get_payload());
    std::cout << "Incoming packet, OPCode:";
    int opcode = j["op"];
    std::cout << opcode << "\nData: " << msg->get_payload() << "\n4";
    if (opcode == 10){
        hello_packet = json::parse(msg->get_payload());
        con->send(get_identify_packet());
    } else {
        // handle_command();
    }
    packet_counter++;
}

// void background_heartbeat(){
//     // while (!packet_counter){}
//     // int delay = hello_packet["d"]["heartbeat_interval"];
//     // while (true){
//     //     std::cout << "Sleeping for " << delay << " miliseconds" << std::endl;
//     //     std::this_thread::sleep_for(std::chrono::milliseconds(delay));
//     // }
// }


bool verify_subject_alternative_name(const char * hostname, X509 * cert) {
    STACK_OF(GENERAL_NAME) * san_names = NULL;

    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }

    int san_names_count = sk_GENERAL_NAME_num(san_names);

    bool result = false;

    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);

        if (current_name->type != GEN_DNS) {
            continue;
        }

        const char * dns_name = (char *) ASN1_STRING_data(current_name->d.dNSName);

        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);

    return result;
}

bool verify_common_name(const char * hostname, X509 * cert) {
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }

    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }

    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }

    const char * common_name_str = (char *) ASN1_STRING_data(common_name_asn1);

    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }

    return (strcasecmp(hostname, common_name_str) == 0);
}


bool verify_certificate(const char * hostname, bool preverified, boost::asio::ssl::verify_context& ctx) {
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());

        if (verify_subject_alternative_name(hostname, cert)) {
            return true;
        } else if (verify_common_name(hostname, cert)) {
            return true;
        } else {
            return false;
        }
    }

    return preverified;
}

context_ptr on_tls_init(const char* hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);


        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        ctx->set_verify_callback(bind(&verify_certificate, hostname, ::_1, ::_2));

        ctx->load_verify_file("ca-chain.cert.pem");
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

int handle_gateway(std::string uri, std::string& token) {
    client c;
    tkn = token;
    std::string hostname = "discord.gg";

    try {
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        c.init_asio();

        c.set_message_handler(&on_message);
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));

        websocketpp::lib::error_code ec;
        con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        c.connect(con);

        c.get_alog().write(websocketpp::log::alevel::app, "Connecting to " + uri);
        
        // auto heartbeat_thread = std::thread{background_heartbeat};

        c.run();
        // heartbeat_thread.join();

    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}