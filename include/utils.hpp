#pragma once
#include "bot.hpp"
#include "result.hpp"

#include <nlohmann/json.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace discord {

    namespace utils {
        template <typename S, typename F>
        inline std::shared_ptr<S> get(std::vector<std::shared_ptr<S>> &iterable,
                                      F const &callable) {
            for (auto const &each : iterable) {
                if (callable(each)) {
                    return each;
                }
            }
            return nullptr;
        }

        template <template <typename...> typename T, typename S, typename F>
        inline const S *get(T<S> const &iterable, F const &callable) {
            for (auto const &each : iterable) {
                if (callable(each)) {
                    return &each;
                }
            }
            return nullptr;
        }

        template <template <typename...> typename T, typename S, typename F>
        inline S *get(T<S> &iterable, F const &callable) {
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
    inline std::vector<T> &from_json_array(nlohmann::json const &j,
                                           std::vector<T> &vec) {
        for (auto const &each : j) {
            vec.emplace_back(each);
        }
        return vec;
    }

    template <typename T, typename Uy, typename... Tys>
    inline std::vector<T> from_json_array(nlohmann::json const &j, Uy &&key,
                                          Tys &&... args) {
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
    inline std::vector<T> from_json_array_special(nlohmann::json const &j,
                                                  Tys &&... args) {
        std::vector<T> return_vec{};
        for (const auto &it : j) {
            return_vec.emplace_back(it, std::forward<Tys>(args)...);
        }
        return return_vec;
    }

    snowflake to_sf(nlohmann::json const &sf);

    snowflake to_sf(std::string sf);

    template <typename S>
    inline void format_slice(std::string const &input_str,
                             std::stringstream &output_str, int &start_index,
                             S var) {
        long unsigned int index = input_str.find('%', start_index);
        if (index == std::string::npos) {
            return;
        }
        output_str << input_str.substr(start_index, index - start_index) << var;
        start_index = index + 1;
    }

    template <typename... T>
    inline std::string format(std::string const &str, T... args) {
        assert(sizeof...(args) == std::count(str.begin(), str.end(), '%') &&
               "Amount of % does not match amount of arguments");
        std::stringstream output_str;
        int start_index = 0;
        ((format_slice(str, output_str, start_index, std::forward<T>(args))), ...);
        output_str << str.substr(start_index, str.length());
        return output_str.str();
    }

    std::string get_cdn_url();

    template <typename T>
    inline T get_value(nlohmann::json const &j, const char *s, T default_value) {
        return j.contains(s) ? (j[s].empty() ? default_value : j[s].get<T>())
                             : default_value;
    }

    std::string get_value(nlohmann::json const &j, const char *s,
                          const char *default_value);

    nlohmann::json get_value(nlohmann::json const &j, const char *s);

    template <typename T>
    inline std::optional<T> get_value_optional(nlohmann::json const &j, const char *key) {
        if (!j.contains(key)) {
            return {};
        }
        if (j[key].is_null()) {
            return {};
        }
        return j[key];
    }

    template <typename T>
    void update_object(nlohmann::json const &j, const char *key, T &var) {
        if (!j.contains(key)) {
            return;
        }
        if (j[key].is_null()) {
            return;
        }
        if constexpr (std::is_same<T, snowflake>::value) {
            var = to_sf(j[key]);
        } else {
            var = j[key];
        }
    }

    template <typename T, typename T2, typename... Tys>
    inline void update_object_bulk(nlohmann::json const &j, T const *one, T2 &two,
                                   Tys &... args) {
        static_assert(sizeof...(args) % 2 == 0,
                      "Invalid amount of arguments passed to update_object_bulk");

        update_object(j, one, two);
        if constexpr (sizeof...(args) > 2) {
            update_object_bulk(j, args...);
        }
    }

    std::string get_channel_link(uint64_t id);

    std::string get_iso_datetime_now();

    std::string get_iso_from_unix(int timestamp);

    std::unordered_map<std::string, std::string> get_default_headers();

    enum permission_type { deny,
                           allow,
                           neutral };

    enum channel_type {
        guild_text_channel,
        dm_channel,
        guild_voice_channel,
        group_dm_channel,
        guild_category_channel,
        guild_news_channel,
        guild_store_channel
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

    enum class response_code {
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

    enum bucket_type { channel,
                       guild,
                       webhook,
                       global };

    template <typename... Tys>
    inline std::string endpoint(std::string endpoint_format, Tys &&... args) {
        endpoint_format =
            endpoint_format[0] == '/' ? endpoint_format : '/' + endpoint_format;
        return format(std::string("https://discordapp.com/api/v6") + endpoint_format,
                      std::forward<Tys>(args)...);
    }

    std::string get_file_extension(std::string const &f_name);

    bool is_image_or_gif(std::string const &f_name);

    std::string image_url_from_type(int asset_t, snowflake some_id,
                                    std::string hash = "",
                                    bool is_animated = false);

    discord::datetime time_from_discord_string(const std::string &tempstr);

    enum request_next_action {
        success,
        resend_request,
        bad_request,
        unauthorized,
        forbidden,
        not_found,
        method_not_allowed,
        too_many_requests,
        server_error
    };

    int handle_resp(int status_code);

    request_response send_request(web::http::method mthd, const std::string &uri,
                                  snowflake obj_id = -1, int bucket_ = global,
                                  nlohmann::json const &j = {});

    std::string read_entire_file(std::ifstream &file);

    std::string encode64(const std::string &val);

    std::string get_os_name();

    std::pair<std::string, std::string> generate_form_data(std::vector<std::pair<std::ifstream, std::string>> &strm, nlohmann::json const &);
}  // namespace discord
