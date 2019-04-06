#pragma once
#include <string>

#include "nlohmann/json.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord{
    using namespace nlohmann;

    template <typename T>
    T get_value(json& j, const char* s, T default_value){
        return j[s].empty() ? default_value : j[s].get<T>();
    }

    inline std::string get_value(json& j, const char* s, const char* default_value){
        return j[s].empty() ? default_value : j[s].get<std::string>();
    }


    inline std::string get_channel_link(long id){
        return "https://discordapp.com/api/v6/channels/" + std::to_string(id) + "/messages";
    }

    inline json send_request(const json& j, const std::list<std::string>& h, const std::string url){
            curlpp::Cleanup clean;
            curlpp::Easy r;
            r.setOpt(new curlpp::options::Url(url));
            r.setOpt(new curlpp::options::HttpHeader(h));
            r.setOpt(new curlpp::options::PostFields(j.dump()));
            r.setOpt(new curlpp::options::PostFieldSize(j.dump().size()));
            std::stringstream response_stream;
            response_stream << r;
            return json::parse(response_stream.str());
    }

    inline std::string get_iso_datetime_now()
    {
        boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
        return boost::posix_time::to_iso_extended_string(t) + "Z";
    }

    inline std::string get_iso_from_unix(int timestamp){
        boost::posix_time::ptime result = boost::posix_time::from_time_t(timestamp);
        return boost::posix_time::to_iso_extended_string(result) + "Z";
    }

    inline std::unordered_map<std::string, int> permission_overwrites{
        {"CREATE_INSTANT_INVITE", 0x1},
        {"KICK_MEMBERS", 0x2},
        {"BAN_MEMBERS", 0x4},
        {"ADMINISTRATOR", 0x8},
        {"MANAGE_CHANNELS", 0x10},
        {"MANAGE_GUILD", 0x20},
        {"ADD_REACTIONS", 0x40},
        {"VIEW_AUDIT_LOG", 0x80},
        {"VIEW_CHANNEL", 0x400},
        {"SEND_MESSAGES", 0x800},
        {"SEND_TTS_MESSAGES", 0x1000},
        {"MANAGE_MEMBERS", 0x2000},
        {"EMBED_LINKS", 0x4000},
        {"ATTACH_FILES", 0x8000},
        {"READ_MESSAGE_HISTORY", 0x10000},
        {"MENTION_EVERYONE", 0x20000},
        {"USE_EXTERNAL_EMOJIS", 0x40000},
        {"CONNECT", 0x100000},
        {"SPEAK", 0x200000},
        {"MUTE_MEMBERS", 0x400000},
        {"DEAFEN_MEMBERS", 0x800000},
        {"MOVE_MEMBERS", 0x1000000},
        {"USE_VAD", 0x2000000},
        {"PRIORITY_SPEAKER", 0x100},
        {"CHANGE_NICKNAME", 0x4000000},
        {"MANAGE_NICKNAMES", 0x8000000},
        {"MANAGE_ROLES", 0x10000000},
        {"MANAGE_WEBHOOKS", 0x20000000},
        {"MANAGE_EMOJIS", 0x40000000}
    };
}