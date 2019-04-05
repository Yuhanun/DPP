#pragma once
#include <discord.hpp>
#include <nlohmann/json.hpp>
#include "member.hpp"
#include "channel.hpp"


namespace discord {
    using namespace nlohmann;
    class Message : public discord::Object {
    public:
        Message() = default;
        Message(discord_id id) : discord::Object(id)
        {}

        static Message from_sent_message(std::string data, std::string token){
            auto j = json::parse(data);
            auto m = Message{};
            m.tts = j["tts"];
            m.timestamp = j["timestamp"];
            m.mention_everyone = j["mention_everyone"];
            m.id = std::stoll( j["id"].get<std::string>() );
            m.author = discord::Member{ std::stoll(j["author"]["id"].get<std::string>()) };
            m.channel = discord::Channel{ std::stoll(j["channel_id"].get<std::string>()) };
            m.content = j["content"];
            m.type = j["type"];
            m.token = token;
            return m;
        }

        std::string get_delete_url(){
            return std::string("https://discordapp.com/api/v6/channels/") + std::to_string(channel.id) + std::string("/messages/") + std::to_string(id);
        }

        json remove(){
            const std::list<std::string> h = {
                { "Authorization: Bot " + token },
                { "Content-Type: application/json" },
                { "User-Agent: DiscordPP (C++ discord library)" },
                { "Connection: keep-alive" }
            };

            curlpp::Cleanup clean;
            curlpp::Easy r;
            r.setOpt(new curlpp::options::CustomRequest{"DELETE"});
            r.setOpt(new curlpp::options::Url(get_delete_url()));
            r.setOpt(new curlpp::options::HttpHeader(h));
            std::stringstream response_stream;
            response_stream << r;
            std::cout << response_stream.str() << std::endl;
            return response_stream.str().length() > 1 ? json::parse("{}") : json::parse(response_stream.str());
        }


        int type;

        bool tts;
        bool pinned;
        bool mention_everyone;

        discord_id id;

        std::string content;
        std::string timestamp;
        // std::string edited_timestamp;
        
        discord::Member author;
        discord::Channel channel;
        
        // std::vector<std::string> mentions;
        // std::vector<discord::Role> mentioned_roles;
        // std::vector<type> attachments; 
        // std::vector<embed> embeds;
    
    private:
        std::string token;


    };
};