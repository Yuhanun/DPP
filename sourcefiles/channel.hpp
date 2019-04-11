#pragma once
#include <discord.hpp>
#include "utility.hpp"
#include "permissions.hpp"

discord::Channel::Channel(snowflake id) : discord::Object(id)
{}

discord::Channel::Channel(std::string guild_create, snowflake guild_id){
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
            break;
        }
    }
    for (auto& each : data["permission_overwrites"]){
        overwrites.push_back(discord::PermissionOverwrites{ each["allow"].get<int>(), std::stoul(each["id"].get<std::string>()), each["type"].get<std::string>()});
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

std::string discord::Channel::get_bulk_delete_url(){
    return format("%/channels/%/messages/bulk-delete", get_api(), id);
}

std::string discord::Channel::get_get_messages_url(int limit){
    return format("%/channels/%/messages?limit=%", get_api(), id, limit);
}

void discord::Channel::bulk_delete(std::vector<discord::Message>& m){
    const std::list<std::string> h = {
        { "Authorization: Bot " + discord::Channel::bot->token },
        { "Content-Type: application/json" },
        { "User-Agent: DiscordPP (C++ discord library)" },
        { "Connection: keep-alive" }
    };
    json array = json::array();
    for (auto const& each : m){
        array.push_back(each.id);
    }
    json data = json();
    data["messages"] = array;
    discord::send_request(data, h, get_bulk_delete_url());
}

std::vector<discord::Message> discord::Channel::get_messages(int limit){
    std::vector<discord::Message> return_vec;
    limit = limit < 1 || limit > 100 ? 50 : limit;
    const std::list<std::string> h = {
        { "Authorization: Bot " + discord::Channel::bot->token },
        { "Content-Type: application/json" },
        { "User-Agent: DiscordPP (C++ discord library)" },
        { "Connection: keep-alive" }
    };

    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(get_get_messages_url(limit)));
    request.setOpt(new curlpp::options::HttpHeader(h));
    std::stringstream reply;
    reply << request;
    auto data = json::parse(reply.str());
    for (auto& each : data){
        return_vec.push_back(discord::Message::from_sent_message(each.dump(), bot));
    }
    return return_vec;
}