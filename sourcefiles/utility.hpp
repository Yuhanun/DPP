#pragma once
#include <string>

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord {
    namespace utils {
        template <typename S, typename F>
        S *get(std::vector<std::unique_ptr<S>> &iterable, F const &callable) {
            for (auto const &each : iterable) {
                if (callable(each)) {
                    return each.get();
                }
            }
            return nullptr;
        }

        template <template <typename...> typename T, typename S, typename F>
        const S *get(T<S> const &iterable, F const &callable) {
            for (auto const &each : iterable) {
                if (callable(each)) {
                    return &each;
                }
            }
            return nullptr;
        }

        template <template <typename...> typename T, typename S, typename F>
        S *get(T<S> &iterable, F const &callable) {
            for (auto &each : iterable) {
                if (callable(each)) {
                    return &each;
                }
            }
            return nullptr;
        }
    }  // namespace utils

    template <typename T>
    inline std::vector<T> &from_json_array(nlohmann::json const &j, std::vector<T> &vec) {
        for (auto const &each : j) {
            vec.emplace_back(each);
        }
        return vec;
    }

    template <typename T, typename Uy, typename... Tys>
    inline std::vector<T> from_json_array(nlohmann::json const &j, Uy &&key, Tys &&... args) {
        std::vector<T> return_vec{};
        if (j.is_null() or not j.contains(key))
            return return_vec;
        for (const auto &it : j[key]) {
            return_vec.emplace_back(it, std::forward<Tys>(args)...);
        }
        return return_vec;
    }

    template <typename T>
    inline std::vector<T> from_json_array(nlohmann::json const &j) {
        std::vector<T> return_vec{};
        for (const auto &it : j) {
            return_vec.emplace_back(it);
        }
        return return_vec;
    }

    inline snowflake to_sf(nlohmann::json const &sf) {
        return std::stoul(sf.get<std::string>());
    }

    inline snowflake to_sf(std::string sf) {
        return std::stoul(sf);
    }

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
        assert(sizeof...(args) == std::count(str.begin(), str.end(), '%') && "Amount of % does not match amount of arguments");
        std::stringstream output_str;
        int start_index = 0;
        ((format_slice(str, output_str, start_index, args)), ...);
        output_str << str.substr(start_index, str.length());
        return output_str.str();
    }

    inline std::string get_api() {
        return "https://discordapp.com/api/v6";
    }

    inline std::string get_image_base_url() {
        return "https://cdn.discordapp.com/";
    }

    template <typename T>
    T get_value(nlohmann::json const &j, const char *s, T default_value) {
        return j.contains(s) ? (j[s].empty() ? default_value : j[s].get<T>()) : default_value;
    }

    inline std::string get_value(nlohmann::json const &j, const char *s, const char *default_value) {
        return j.contains(s) ? (j[s].empty() ? default_value : j[s].get<std::string>()) : default_value;
    }

    inline nlohmann::json get_value(nlohmann::json const &j, const char *s) {
        return j.contains(s) ? j[s] : nlohmann::json{};
    }

    inline std::string get_channel_link(uint64_t id) {
        return format("%/channels/%/messages", get_api(), id);
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


    inline cpr::Header get_default_headers() {
        return cpr::Header{
            { "Authorization", format("Bot %", discord::detail::bot_instance->token) },
            { "Content-Type", "application/json" },
            { "User-Agent", "DiscordPP (http://www.github.com/yuhanun/dpp, 0.0.0)" },
            { "Connection", "keep-alive" }
        };
    }

    enum object_type {
        role,
        member
    };

    enum permission_type {
        deny,
        allow,
        neutral
    };

    enum channel_type {
        guild_text_channel,
        dm_channel,
        guild_voice_channel,
        group_dm_channel,
        guild_category_channel,
        guild_news_channel,
        guild_store_channel
    };

    enum request_method {
        Get,
        Put,
        Post,
        Patch,
        Delete
    };

    discord::datetime time_from_discord_string(const std::string &tempstr) {
        if (tempstr.empty()) {
            return boost::posix_time::ptime{};
        }
        auto time_str = tempstr.substr(0, tempstr.size() - 6);
        time_str[10] = ' ';
        return boost::posix_time::time_from_string(time_str);
    }

    template <size_t method>
    inline nlohmann::json send_request(const nlohmann::json &j, const cpr::Header &h, const std::string &uri) {
        auto session = cpr::Session();
        auto url = cpr::Url{ uri };
        auto body = cpr::Body{ j.dump() };

        cpr::Response response;
        if (method == request_method::Get) {
            response = cpr::Get(url, h);
        } else if (method == request_method::Post) {
            response = cpr::Post(url, h, body);
        } else if (method == request_method::Put) {
            response = cpr::Put(url, h, body);
        } else if (method == request_method::Delete) {
            response = cpr::Delete(url, h, body);
        } else if (method == request_method::Patch) {
            response = cpr::Patch(url, h, body);
        }

        std::cout << response.text << std::endl;
        auto j_resp = response.text.length() ? nlohmann::json::parse(response.text) : nlohmann::json({});
        if (j_resp.contains("message")) {
            if (j_resp["message"].get<std::string>() == "You are being rate limited.") {
                std::this_thread::sleep_for(std::chrono::milliseconds(j_resp["retry_after"].get<int>() + 1));
                return send_request<method>(j, h, uri);
            }
        }
        return j_resp;
    }

    inline std::string get_os_name() {
#ifdef _WIN32
        return "Windows 32-bit";
#elif _WIN64
        return "Windows 64-bit";
#elif __unix || __unix__
        return "Unix";
#elif __APPLE__ || __MACH__
        return "Mac OSX";
#elif __linux__
        return "Linux";
#elif __FreeBSD__
        return "FreeBSD";
#else
        return "Other";
#endif
    }

}  // namespace discord