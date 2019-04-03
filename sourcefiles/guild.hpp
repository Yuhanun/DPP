#include <discord.hpp>
#include <nlohmann/json.hpp>

class discord::Channel;
class discord::Role;
class discord::Member;
class discord::Emoji;

namespace discord {
    
    using namespace nlohmann;

    class Guild : public discord::Object {
    public:
        Guild() : discord::Object()
        {}

        Guild(discord_id id) : discord::Object(id) {

        }

        Guild(json& guild_create_event){
            // Guild g{};
            // json guild = guild_create_event["d"];

            // g.splash = test_null(guild["splash"]);
            // g.mfa_level = guild["mfa_level"];
            // g.afk_timeout = guild["afk_timeout"];
            // g.member_count = guild["member_count"];
            // g.verification_level = guild["verification_level"];
            // g.explicit_content_filter = guild["explicit_content_filter"];

            // g.large = guild["large"];
            // g.unavailable = guild["unavailable"];

            // g.id = std::stoll(guild["id"]);
            // g.application_id = std::stoll(test_null(guild["application_id"]));

            // g.name = guild["name"];
            // g.icon = test_null(guild["icon"]);
            // g.region = guild["region"];
            // g.banner = test_null(guild["banner"]);
            // g.created_at = guild["joined_at"];
            // g.vanity_url_code = test_null(guild["vanity_url_code"]);
        }

        template <typename T>
        static T test_null(T&& value){
            return std::forward<T>(value).is_null() ? T{} : std::forward<T>(value);
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