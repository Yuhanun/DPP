#pragma once
#include <discord.hpp>
#include <nlohmann/json.hpp>
#include "member.hpp"
#include "channel.hpp"


discord::Message::Message(discord_id id) : discord::Object(id)
{}

discord::Message discord::Message::from_sent_message(std::string data){
    auto j = json::parse(data);
    auto m = Message{};
    if (j.contains("message")){
        m.error_code = j["code"];
        m.error = j["message"];
        m.sent = false;
    } else {
        m.sent = true;
        m.tts = j["tts"];
        m.timestamp = j["timestamp"];
        m.mention_everyone = j["mention_everyone"];
        m.id = std::stoll( j["id"].get<std::string>() );
        m.author = discord::Member{ std::stoll(j["author"]["id"].get<std::string>()) };
        m.channel = discord::Channel{ std::stoll(j["channel_id"].get<std::string>()) };
        m.content = j["content"];
        m.type = j["type"];
    }
    return m;
}

std::string discord::Message::get_delete_url(){
    return std::string("https://discordapp.com/api/v6/channels/") + std::to_string(channel.id) + std::string("/messages/") + std::to_string(id);
}

nlohmann::json discord::Message::remove(){
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
