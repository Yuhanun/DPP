#pragma once
#include <string>

#include "nlohmann/json.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord {
    using namespace nlohmann;

    namespace utils {
        template <typename S, typename F>
        S *get(std::vector<std::unique_ptr<S>> &iterable, F callable) {
            for (auto &each : iterable) {
                if (callable(each)) {
                    return each.get();
                }
            }
            return nullptr;
        }
    };  // namespace utils

    template <typename T>
    T get_value(json &j, const char *s, T default_value) {
        return j[s].empty() ? default_value : j[s].get<T>();
    }

    inline std::string get_value(json &j, const char *s, const char *default_value) {
        return j[s].empty() ? default_value : j[s].get<std::string>();
    }

    inline std::string get_channel_link(long id) {
        return "https://discordapp.com/api/v6/channels/" + std::to_string(id) +
               "/messages";
    }

    template <size_t method>
    inline std::future<json> send_request(const json &j, const cpr::Header &h, const std::string &uri, const bool &wait_for = true) {
        return std::async(std::launch::async, [&]() {
            auto session = cpr::Session();
            auto url = cpr::Url{ uri };
            auto body = cpr::Body{ j.dump() };

            std::future<cpr::Response> response;
            if (method == request_type::Get) {
                response = cpr::GetAsync(url, h);
            } else if (method == request_type::Post) {
                response = cpr::PostAsync(url, h, body);
            } else if (method == request_type::Put) {
                response = cpr::PutAsync(url, h, body);
            } else if (method == request_type::Delete) {
                response = cpr::DeleteAsync(url, h, body);
            } else if (method == request_type::Patch) {
                response = cpr::PatchAsync(url, h, body);
            }

            if (wait_for) {
                response.wait();
                auto r = response.get().text;
                return r.length() > 0 ? json::parse(r) : json({});
            } else {
                return json({});
            }
        });
    }

    inline std::string get_iso_datetime_now() {
        boost::posix_time::ptime t =
            boost::posix_time::microsec_clock::universal_time();
        return boost::posix_time::to_iso_extended_string(t) + "Z";
    }

    inline std::string get_iso_from_unix(int timestamp) {
        boost::posix_time::ptime result = boost::posix_time::from_time_t(timestamp);
        return boost::posix_time::to_iso_extended_string(result) + "Z";
    }

    inline std::unordered_map<std::string, int> permission_overwrites{
        { "CREATE_INSTANT_INVITE", 0x1 },
        { "KICK_MEMBERS", 0x2 },
        { "BAN_MEMBERS", 0x4 },
        { "ADMINISTRATOR", 0x8 },
        { "MANAGE_CHANNELS", 0x10 },
        { "MANAGE_GUILD", 0x20 },
        { "ADD_REACTIONS", 0x40 },
        { "VIEW_AUDIT_LOG", 0x80 },
        { "VIEW_CHANNEL", 0x400 },
        { "SEND_MESSAGES", 0x800 },
        { "SEND_TTS_MESSAGES", 0x1000 },
        { "MANAGE_MEMBERS", 0x2000 },
        { "EMBED_LINKS", 0x4000 },
        { "ATTACH_FILES", 0x8000 },
        { "READ_MESSAGE_HISTORY", 0x10000 },
        { "MENTION_EVERYONE", 0x20000 },
        { "USE_EXTERNAL_EMOJIS", 0x40000 },
        { "CONNECT", 0x100000 },
        { "SPEAK", 0x200000 },
        { "MUTE_MEMBERS", 0x400000 },
        { "DEAFEN_MEMBERS", 0x800000 },
        { "MOVE_MEMBERS", 0x1000000 },
        { "USE_VAD", 0x2000000 },
        { "PRIORITY_SPEAKER", 0x100 },
        { "CHANGE_NICKNAME", 0x4000000 },
        { "MANAGE_NICKNAMES", 0x8000000 },
        { "MANAGE_ROLES", 0x10000000 },
        { "MANAGE_WEBHOOKS", 0x20000000 },
        { "MANAGE_EMOJIS", 0x40000000 }
    };

    template <typename S>
    inline void format_slice(std::string const &input_str, std::stringstream &output_str, int &start_index, S var) {
        long unsigned int index = input_str.find('%', start_index);
        if (index == std::string::npos) {
            return;
        }
        output_str << input_str.substr(start_index, index - start_index) << var;
        start_index = index + 1;
    }

    template <typename... T>
    inline std::string format(std::string const &str, T... args) {
        std::stringstream output_str;
        int start_index = 0;
        ((format_slice(str, output_str, start_index, args)), ...);
        output_str << str.substr(start_index, str.length());
        return output_str.str();
    }

    inline std::string get_api() {
        return "https://discordapp.com/api/v6";
    }

    inline cpr::Header get_default_headers() {
        return cpr::Header{
            { "Authorization", format("Bot %", discord::bot_instance->token) },
            { "Content-Type", "application/json" },
            { "User-Agent", "DiscordPP (C++ discord library)" },
            { "Connection", "keep-alive" }
        };
    }

    enum object_type { role,
                       member };

    enum permission_type { deny,
                           allow,
                           neutral };

    enum channel_type { TextChannel,
                        VoiceChannel,
                        CategoryChannel };

    enum request_method {
        Get,
        Put,
        Post,
        Patch,
        Delete
    };
}  // namespace discord