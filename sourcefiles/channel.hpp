#pragma once
#include <discord.hpp>
#include "utility.hpp"
#include "permissions.hpp"
#include <invite.hpp>

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
    for (auto const& v_guild : discord::bot_instance->guilds){
        if (v_guild->id == guild_id){
            guild = v_guild.get();
            break;
        }
    }
    for (auto& each : data["permission_overwrites"]){
        int type = each["type"].get<std::string>() == "role" ? role : member;
        overwrites.push_back(discord::PermissionOverwrites{ 
            each["allow"].get<int>(),
            each["deny"].get<int>(),
            std::stoul(each["id"].get<std::string>()),
            type
        });
    }
    name = data["name"];
    position = data["position"];
    id = std::stoul( data["id"].get<std::string>() );
    // type = data["type"];
}

discord::Message discord::Channel::send(std::string content, bool tts){
    return discord::bot_instance->send_message(id, content, tts);
}

discord::Message discord::Channel::send(EmbedBuilder embed, bool tts, std::string content){
    json j = json(
        {
            {"embed", embed.to_json()},
            {"tts", tts}
        }
    );

    if (content != ""){
        j["content"] = content;
    }

    return discord::bot_instance->send_message(id, j, tts);
}

std::string discord::Channel::get_bulk_delete_url(){
    return format("%/channels/%/messages/bulk-delete", get_api(), id);
}

std::string discord::Channel::get_get_messages_url(int limit){
    return format("%/channels/%/messages?limit=%", get_api(), id, limit);
}

void discord::Channel::bulk_delete(std::vector<discord::Message>& m){
    json array = json::array();
    for (auto const& each : m){
        array.push_back(each.id);
    }
    json data = json();
    data["messages"] = array;
    discord::send_request(data, get_default_headers(), get_bulk_delete_url());
}

std::vector<discord::Message> discord::Channel::get_messages(int limit){
    std::vector<discord::Message> return_vec;
    limit = limit < 1 || limit > 100 ? 50 : limit;


    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(get_get_messages_url(limit)));
    request.setOpt(new curlpp::options::HttpHeader(get_default_headers()));
    std::stringstream reply;
    reply << request;
    auto data = json::parse(reply.str());
    for (auto& each : data){
        return_vec.push_back(discord::Message::from_sent_message(each.dump()));
    }
    return return_vec;
}

std::string discord::Channel::get_channel_edit_url(){
    return format("%/channels/%", get_api(), id);
}

std::string discord::Channel::get_delete_url(){
    return format("%/channels/%", get_api(), id);
}

void discord::Channel::edit(json& j){
    send_request(j, get_default_headers(), get_channel_edit_url(), "PATCH");
}

void discord::Channel::remove(){
    discord::send_request(json({}), get_default_headers(), get_delete_url(), "DELETE");
}

discord::Message discord::Channel::get_message(snowflake id){
    return discord::Message::from_sent_message(
        discord::send_request(json({}), get_default_headers(), get_get_message_url(id), "GET").dump());
}

std::vector<discord::Invite> discord::Channel::get_invites(){
    std::vector<discord::Invite> return_vec;
    auto response = discord::send_request(json({}), get_default_headers(), get_channel_invites_url(), "GET");
    for (auto const& each : response){
        return_vec.push_back(discord::Invite{each.dump()});
    }
    return return_vec;
}

discord::Invite discord::Channel::create_invite(int max_age, int max_uses, bool temporary, bool unique){
    json data = json({
        {"max_age", max_age},
        {"max_uses", max_uses},
        {"temporary", temporary},
        {"unique", unique}
    });
    return discord::Invite{ discord::send_request(data, get_default_headers(), get_create_invite_url(), "POST").dump() };
}

std::vector<discord::Message> discord::Channel::get_pins(){
    std::vector<discord::Message> message_vec;
    auto reply = discord::send_request(json({}), get_default_headers(), get_pins_url(), "GET");
    for (auto const& each : reply){
        message_vec.push_back( discord::Message::from_sent_message(each.dump()));
    }
    return message_vec;
}

void discord::Channel::remove_permissions(discord::Object const& obj){
    discord::send_request(json({}), get_default_headers(), get_delete_channel_permission_url(obj), "DELETE");
}

void discord::Channel::typing(){
    discord::send_request(json({}), get_default_headers(), get_typing_url(), "POST");
}

std::string discord::Channel::get_get_message_url(snowflake m_id){
    return format("%/channels/%/messages/%", get_api(), id, m_id);
}

std::string discord::Channel::get_channel_invites_url(){
    return format("%/channels/%/invites", get_api(), id);
}

std::string discord::Channel::get_create_invite_url(){
    return format("%/channels/%/invites", get_api(), id);
}

std::string discord::Channel::get_delete_channel_permission_url(discord::Object const& obj){
    return format("%/channels/%/permissions/%", id, obj.id);
}

std::string discord::Channel::get_typing_url(){
    return format("%/channels/%/typing", get_api(), id);
}

std::string discord::Channel::get_pins_url(){
    return format("%/channels/%/pins", get_api(), id);
}