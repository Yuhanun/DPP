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
            parent_id = std::stol( data["parent_id"].get<std::string>() );
        }
        bitrate = data["bitrate"];
        user_limit = data["user_limit"];
    } else {
        type = channel_type::TextChannel;
        parent_id = std::stol( data["parent_id"].get<std::string>() );
        rate_limit_per_user = data["rate_limit_per_user"];
        topic = discord::get_value(data, "topic", "");
    }

    name = data["name"];
    position = data["position"];
    id = std::stol( data["id"].get<std::string>() );
    // type = data["type"];
}

discord::Message discord::Channel::send(std::string content){
    return discord::Channel::bot->send_message(id, content);
}

discord::Message discord::Channel::send(EmbedBuilder embed, std::string content){
    const std::list<std::string> h = {
        { "Authorization: Bot " + discord::Channel::bot->token },
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

    return discord::Channel::bot->send_message(id, j.dump());
}

