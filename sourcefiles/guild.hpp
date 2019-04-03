#include <discord.hpp>
#include <nlohmann/json.hpp>
#include <channel.hpp>
#include <role.hpp>
#include <member.hpp>
#include <emoji.hpp>


namespace discord {
    
    using namespace nlohmann;

    class Guild : public discord::Object {
        Guild(discord_id id) : discord::Object(id) {

        }

        static Guild from_guild_create(json& event){
            
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