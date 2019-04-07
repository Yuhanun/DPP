#pragma once
#include <discord.hpp>
#include "utility.hpp"

discord::Channel::Channel(discord_id id) : discord::Object(id)
{}

discord::Channel::Channel(std::string guild_create, discord_id guild_id){
    json data = json::parse(guild_create);
    if (data.contains("bitrate")) {
        if (!data.contains("parent_id")){
            type = channel_type::CategoryChannel;
        } else {
            type = channel_type::VoiceChannel;
            parent_id = std::stoul( get_value(data, "parent_id", "0") );
        }
        bitrate = data["bitrate"];
        user_limit = data["user_limit"];
    } else {
        type = channel_type::TextChannel;
        parent_id = std::stoul( get_value(data, "parent_id", "0") );
        rate_limit_per_user = get_value(data, "rate_limit_per_user", 0);
        topic = discord::get_value(data, "topic", "");
    }

    guild = nullptr;
    for (auto const& v_guild : bot->guilds){
        if (v_guild->id == guild_id){
            guild = v_guild.get();
        }
    }
    name = data["name"];
    position = data["position"];
    id = std::stoul( data["id"].get<std::string>() );
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

    return discord::Channel::bot->send_message(id, j);
}

