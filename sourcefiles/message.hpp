#pragma once
#include <discord.hpp>
#include <nlohmann/json.hpp>
#include "member.hpp"
#include "channel.hpp"


discord::Message::Message(snowflake id) 
    : id{id}
{}

discord::Message discord::Message::from_sent_message(std::string data, discord::Bot* bot){
    auto j = json::parse(data);
    auto m = Message{};
    m.token = bot->token;
    if (j.contains("message")){
        m.error_code = j["code"];
        m.error = j["message"];
        m.sent = false;
    } else {
        snowflake sender_id = std::stoul(j["author"]["id"].get<std::string>());
        m.sent = true;
        m.tts = j["tts"];
        m.timestamp = j["timestamp"];
        m.mention_everyone = j["mention_everyone"];
        m.id = std::stoul( j["id"].get<std::string>() );
        snowflake channel_id = std::stoul(j["channel_id"].get<std::string>());

        for (auto const& chan : bot->channels){
            if (chan->id == channel_id){
                m.channel = *(chan.get());
                break;
            }
        }

        for (auto const& member : m.channel.guild->members){
            if (member.id == sender_id){
                m.author = member;
                break;
            }
        }


        m.content = j["content"];
        m.type = j["type"];
    }
    return m;
}

std::string discord::Message::get_delete_url(){
    return std::string("https://discordapp.com/api/v6/channels/") + std::to_string(channel.id) + std::string("/messages/") + std::to_string(id);
}

void discord::Message::remove(){
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
    r.perform();
}
