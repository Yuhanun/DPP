#pragma once
#include <discord.hpp>
#include "utility.hpp"

discord::Channel::Channel(discord_id id) : discord::Object(id)
{}

discord::Channel::Channel(std::string guild_create){
    json data = json::parse(guild_create);
    if (data.contains("bitrate")) {
        if (!data.contains("parent_id")){
            type = channel_type::CategoryChannel;
        } else {
            type = channel_type::VoiceChannel;
            parent_id = std::stoll( data["parent_id"].get<std::string>() );
        }
        bitrate = data["bitrate"];
        user_limit = data["user_limit"];
    } else {
        type = channel_type::TextChannel;
        parent_id = std::stoll( data["parent_id"].get<std::string>() );
        rate_limit_per_user = data["rate_limit_per_user"];
        topic = discord::get_value(data, "topic", "");
    }

    name = data["name"];
    position = data["position"];
    id = std::stoll( data["id"].get<std::string>() );
    // type = data["type"];
}

discord::Message discord::Channel::send(std::string content){
    const std::list<std::string> h = {
        { "Authorization: Bot " + token },
        { "Content-Type: application/json" },
        { "User-Agent: DiscordPP (C++ discord library)" },
        { "Connection: keep-alive" }
    };
    json j = json(
        {
            {"content", content},
            {"tts", false}
        }
    );

    json response = send_request(j, h, get_channel_link(id));
    return discord::Message::from_sent_message(response.dump());
}

discord::Message discord::Channel::send(EmbedBuilder embed, std::string content){
    const std::list<std::string> h = {
        { "Authorization: Bot " + token },
        { "Content-Type: application/json" },
        { "User-Agent: DiscordPP (C++ discord library)" },
        { "Connection: keep-alive" }
    };
    json j = json(
        {
            {"embed", embed.to_json()},
            {"tts", false}
        }
    );

    if (content != ""){
        j["content"] = content;
    }

    json response = send_request(j, h, get_channel_link(id));
    return discord::Message::from_sent_message(response.dump());
}

