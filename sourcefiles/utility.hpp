#pragma once
#include <string>

#include "cpr/cpr.h"
#include "discord.hpp"
#include "nlohmann/json.hpp"

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord {
    namespace utils {
        template <typename S, typename F>
        std::shared_ptr<S> get(std::vector<std::shared_ptr<S>> &iterable, F const &callable) {
            for (auto const &each : iterable) {
                if (callable(each)) {
                    return each;
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
            return_vec.push_back({ it, std::forward<Tys>(args)... });
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

    template <typename T, typename... Tys>
    inline std::vector<T> from_json_array_special(nlohmann::json const &j, Tys &&... args) {
        std::vector<T> return_vec{};
        for (const auto &it : j) {
            return_vec.emplace_back(it, std::forward<Tys>(args)...);
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
        ((format_slice(str, output_str, start_index, std::forward<T>(args))), ...);
        output_str << str.substr(start_index, str.length());
        return output_str.str();
    }

    inline std::string get_cdn_url() {
        return "https://cdn.discordapp.com";
    }

    template <typename T>
    inline T get_value(nlohmann::json const &j, const char *s, T default_value) {
        return j.contains(s) ? (j[s].empty() ? default_value : j[s].get<T>()) : default_value;
    }

    inline std::string get_value(nlohmann::json const &j, const char *s, const char *default_value) {
        return j.contains(s) ? (j[s].empty() ? default_value : j[s].get<std::string>()) : default_value;
    }

    inline nlohmann::json get_value(nlohmann::json const &j, const char *s) {
        return j.contains(s) ? j[s] : nlohmann::json{};
    }

    template <typename T>
    inline void update_object(nlohmann::json const &j, const char *key, T &var) {
        if (!j.contains(key)) {
            return;
        }
        if (j[key].is_null()) {
            return;
        }

        if constexpr (std::is_same<T, snowflake>::value) { // sometimes wrong
            var = to_sf(j[key]);
        } else {
            var = j[key];
        }
    }

    template <typename T, typename T2, typename... Tys>
    inline void update_object_bulk(nlohmann::json const &j, T const* one, T2& two, Tys&... args) {
        static_assert(sizeof...(args) % 2 == 0, "Invalid amount of arguments passed to update_object_bulk");

        update_object(j, one, two);
        if constexpr(sizeof...(args) > 2) {
            update_object_bulk(j, args...);
        }
    }

    inline std::string get_channel_link(uint64_t id) {
        return format("/channels/%/messages", id);
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

    inline cpr::Header get_default_headers() {
        return cpr::Header{
            { "Authorization", format("Bot %", discord::detail::bot_instance->token) },
            { "Content-Type", "application/json" },
            { "User-Agent", "DiscordBot (http://www.github.com/yuhanun/dpp, 0.0.0)" },
            { "Connection", "keep-alive" }
        };
    }

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

    enum asset_type {
        custom_emoji,
        guild_icon,
        guild_splash,
        guild_banner,
        default_user_avatar,
        user_avatar,
        application_icon,
        application_asset
    };

    enum response_code {
        ok = 200,
        created = 201,
        no_content = 204,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        method_not_allowed = 405,
        too_many_requests = 429,
        server_error = 501,
        gateway_unavailable = 502,
    };

    template <typename... Tys>
    inline std::string endpoint(std::string endpoint_format, Tys &&... args) {
        endpoint_format = endpoint_format[0] == '/' ? endpoint_format : '/' + endpoint_format;
        return format(std::string("https://discordapp.com/api/v6") + endpoint_format, std::forward<Tys>(args)...);
    }

    inline std::string get_file_extension(std::string const &f_name) {
        auto extension = f_name.substr(f_name.find_last_of('.') + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension;
    }

    inline bool is_image_or_gif(std::string const &f_name) {
        auto extension = get_file_extension(f_name);
        return (extension == "png" || extension == "jpeg" || extension == "jpg" || extension == "webp" || extension == "gif");
    }

    inline std::string image_url_from_type(int asset_t, snowflake some_id, std::string hash = "", bool is_animated = false) {
        switch (asset_t) {
            case custom_emoji:
                return format(is_animated ? "%/emojis/%.gif" : "%/emojis/%.png", get_cdn_url(), some_id);
            case guild_icon:
                return format("%/icons/%/%.png", get_cdn_url(), some_id, hash);
            case guild_splash:
                return format("%/splashes/%/%.png", get_cdn_url(), some_id, hash);
            case guild_banner:
                return format("%/banners/%/%.png", get_cdn_url(), some_id, hash);
            case default_user_avatar:
                return format("%/embed/avatars/%.png", get_cdn_url(), some_id % 5);
            case user_avatar:
                return format(is_animated ? "%/avatars/%/%.gif" : "%/avatars/%/%.png", get_cdn_url(), some_id, hash);
            case application_icon:
                return format("%/app-cons/%/%.png", get_cdn_url(), some_id, hash);
            case application_asset:
                return format("%/app-assets/%/%.png", get_cdn_url(), some_id, hash);
            default:
                assert(false && "Invalid asset type");
        }
    }

    discord::datetime time_from_discord_string(const std::string &tempstr) {
        if (tempstr.empty()) {
            return boost::posix_time::ptime{};
        }
        auto time_str = tempstr.substr(0, tempstr.size() - 6);
        time_str[10] = ' ';
        return boost::posix_time::time_from_string(time_str);
    }

    enum class request_next_action {
        resend_request,
        empty_response,
        not_modified,
        bad_request,
        no_auth_header,
        forbidden,
        invalid_method,
        ratelimit,
        gateway_unavailable,
        server_error,
        nothing
    };

    request_next_action handle_http_response(cpr::Response const &resp, nlohmann::json const &parsed_json_resp) {
        switch (resp.status_code) {
            case 200:
                return request_next_action::nothing;
            case 201:
                return request_next_action::nothing;
            case 204:
                return request_next_action::empty_response;
            case 304:
                return request_next_action::not_modified;
            case 400:
                return request_next_action::bad_request;
            case 401:
                return request_next_action::no_auth_header;
            case 403:
                return request_next_action::forbidden;
            case 405:
                return request_next_action::invalid_method;
            case 429:
                std::this_thread::sleep_for(std::chrono::milliseconds(parsed_json_resp["retry_after"].get<int>() + 1));
                return request_next_action::ratelimit;
            case 502:
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                return request_next_action::gateway_unavailable;
            default:
                return request_next_action::server_error;
        }
    }

    template <size_t method>
    inline nlohmann::json send_request(const nlohmann::json &j, const cpr::Header &h, const std::string &uri) {
        auto session = cpr::Session();
        auto url = cpr::Url{ uri };
        auto body = cpr::Body{ j.dump() };

        static_assert(method < 5, "Invalid request method.");

#ifdef __DPP_DEBUG
        std::cout << j.dump(4) << "\n"
                  << uri << std::endl;
#endif

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

        auto j_resp = response.text.length() ? nlohmann::json::parse(response.text) : nlohmann::json({});

#ifdef __DPP_DEBUG
        std::cout << j_resp.dump(4) << std::endl;
#endif
        request_next_action to_handle = handle_http_response(response, j_resp);
        if (to_handle == request_next_action::nothing) {
            return j_resp;
        } else if (to_handle == request_next_action::ratelimit || to_handle == request_next_action::gateway_unavailable) {
            return send_request<method>(j, h, uri);
        } else {
            return j_resp;
        }
    }

    inline std::string read_entire_file(std::string const &filename) {
        std::ifstream ifs(filename);
        return std::string((std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>()));
    }

    inline std::string encode64(const std::string &val) {
        using namespace boost::archive::iterators;
        using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
        return tmp.append((3 - val.size() % 3) % 3, '=');
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
#elif __FreeBSD__
        return "FreeBSD";
#else
        return "Other";
#endif
    }

}  // namespace discord
