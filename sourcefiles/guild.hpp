#pragma once
#include <discord.hpp>
#include <nlohmann/json.hpp>

#include <channel.hpp>
#include <role.hpp>
#include <member.hpp>
#include <emoji.hpp>


namespace discord {
    
    using namespace nlohmann;

    class Guild : public discord::Object {
    public:
        Guild(){}

        Guild(discord_id id) : discord::Object(id) {}

        Guild(std::string guild_create_event){
            json guild = json::parse(guild_create_event)["d"];

            splash = get_value(guild, "splash", 0);
            mfa_level = get_value(guild, "mfa_level", 0);
            afk_timeout = get_value(guild, "afk_timeout", 0);
            member_count = get_value(guild, "member_count", 0);
            verification_level = get_value(guild, "verification_level", 0);
            explicit_content_filter = get_value(guild, "explicit_content_filter", 0);

            large = guild["large"];
            unavailable = guild["unavailable"];

            std::string temp_id = guild["id"];
            id = std::stoll(temp_id);
            std::string temp_app_id = get_value(guild, "application_id", "0");
            application_id = std::stoll(temp_app_id);

            name = guild["name"];
            icon = get_value(guild, "icon", "");
            region = guild["region"];
            banner = get_value(guild, "banner", "");
            created_at = guild["joined_at"];
            vanity_url_code = get_value(guild, "vanity_url_code", "");
        }

        std::string get_value(json& j, const char* s, const char* default_value){
            return j[s].empty() ? default_value : j[s].get<std::string>();
        }

        template <typename T>
        T get_value(json& j, const char* s, T default_value){
            return j[s].empty() ? default_value : j[s].get<T>();
        }

        int splash;
        int mfa_level;
        int afk_timeout;
        int member_count;
        int verification_level;
        int explicit_content_filter;

        bool large;
        bool unavailable;
        
        discord_id id;
        discord_id application_id;

        std::string name;
        std::string icon;
        std::string region;
        std::string banner;
        std::string created_at;
        std::string vanity_url_code;

        std::vector<int> features;
        std::vector<discord::Role> roles;
        std::vector<discord::Emoji> emojis;
        std::vector<discord::Member> members;
        std::vector<discord::Channel> channels;

        discord::Member owner;
        discord::Channel afk_channel;
        discord::Channel system_channel;
    };

};