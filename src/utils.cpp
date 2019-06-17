#include "utils.hpp"
#include "bot.hpp"

namespace discord {

    snowflake to_sf(nlohmann::json const &sf) {
        return std::stoul(sf.get<std::string>());
    }

    snowflake to_sf(std::string sf) {
        return std::stoul(sf);
    }

    std::string get_cdn_url() {
        return "https://cdn.discordapp.com";
    }


    std::string get_value(nlohmann::json const &j, const char *s,
                          const char *default_value) {
        return j.contains(s)
                   ? (j[s].empty() ? default_value : j[s].get<std::string>())
                   : default_value;
    }

    nlohmann::json get_value(nlohmann::json const &j, const char *s) {
        return j.contains(s) ? j[s] : nlohmann::json{};
    }


    std::string get_channel_link(uint64_t id) {
        return format("/channels/%/messages", id);
    }

    std::string get_iso_datetime_now() {
        boost::posix_time::ptime t =
            boost::posix_time::microsec_clock::universal_time();
        return boost::posix_time::to_iso_extended_string(t) + "Z";
    }

    std::string get_iso_from_unix(int timestamp) {
        boost::posix_time::ptime result = boost::posix_time::from_time_t(timestamp);
        return boost::posix_time::to_iso_extended_string(result) + "Z";
    }

    std::unordered_map<std::string, std::string> get_default_headers() {
        return {
            { "Authorization", format("Bot %", discord::detail::bot_instance->token) },
            { "User-Agent", "DiscordBot (http://www.github.com/yuhanun/dpp, 0.0.0)" }
        };
    }

    std::string get_file_extension(std::string const &f_name) {
        auto extension = f_name.substr(f_name.find_last_of('.') + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       ::tolower);
        return extension;
    }

    bool is_image_or_gif(std::string const &f_name) {
        auto extension = get_file_extension(f_name);
        return (extension == "png" || extension == "jpeg" || extension == "jpg" ||
                extension == "webp" || extension == "gif");
    }

    std::string image_url_from_type(int asset_t, snowflake some_id,
                                    std::string hash,
                                    bool is_animated) {
        switch (asset_t) {
            case custom_emoji:
                return format(is_animated ? "%/emojis/%.gif" : "%/emojis/%.png",
                              get_cdn_url(), some_id);
            case guild_icon:
                return format("%/icons/%/%.png", get_cdn_url(), some_id, hash);
            case guild_splash:
                return format("%/splashes/%/%.png", get_cdn_url(), some_id, hash);
            case guild_banner:
                return format("%/banners/%/%.png", get_cdn_url(), some_id, hash);
            case default_user_avatar:
                return format("%/embed/avatars/%.png", get_cdn_url(), some_id % 5);
            case user_avatar:
                return format(is_animated ? "%/avatars/%/%.gif" : "%/avatars/%/%.png",
                              get_cdn_url(), some_id, hash);
            case application_icon:
                return format("%/app-cons/%/%.png", get_cdn_url(), some_id, hash);
            case application_asset:
                return format("%/app-assets/%/%.png", get_cdn_url(), some_id, hash);
            default:
                assert(false && "Invalid asset type");
        }
    }

    boost::posix_time::ptime time_from_discord_string(const std::string &tempstr) {
        if (tempstr.empty()) {
            return boost::posix_time::ptime{};
        }
        auto time_str = tempstr.substr(0, tempstr.size() - 6);
        time_str[10] = ' ';
        return boost::posix_time::time_from_string(time_str);
    }

    int handle_resp(int status_code) {
        if (status_code >= 200 && status_code < 300) {
            return success;
        } else if (status_code == 429) {
            return resend_request;
        } else if (status_code == 401 || status_code == 403) {  // unauthorized
            return status_code;
        } else {
            return status_code;
        }
    }

    pplx::task<Result<nlohmann::json>> send_request(web::http::method mthd, const std::string &uri,
                                  snowflake obj_id, int bucket_,
                                  nlohmann::json const &j) {
        discord::detail::bot_instance->wait_for_ratelimits(obj_id, bucket_);
        web::http::client::http_client client{ { uri } };
        web::http::http_request msg{ mthd };

        if (mthd != methods::GET) {
            msg.set_body(j.dump());
        }

        for (auto const &each : get_default_headers()) {
            msg.headers().add(each.first, each.second);
        }

        return client.request(msg).then([=](http_response resp_val) {
            discord::detail::bot_instance->handle_ratelimits(resp_val.headers(), obj_id, bucket_);
            auto next_action = static_cast<int>(handle_resp(resp_val.status_code()));

            auto response = resp_val.extract_utf8string(true).get();

            auto parsed_json = response.empty() ? nlohmann::json{} : nlohmann::json::parse(response);

#ifdef __DPP_DEBUG
            std::cout << parsed_json.dump(4) << std::endl;
#endif

            if (next_action == success) {
                return OK(parsed_json);
            } else if (next_action == resend_request) {
                return send_request(mthd, uri, obj_id, bucket_, j).get();
            } else {
                return ERR<nlohmann::json>(get_value(parsed_json, "message", ""), j, get_default_headers(), resp_val);
            }
        });
    }

    std::string read_entire_file(std::ifstream &ifs) {
        return std::string((std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>()));
    }

    std::string encode64(const std::string &val) {
        using namespace boost::archive::iterators;
        using It =
            base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
        return tmp.append((3 - val.size() % 3) % 3, '=');
    }

    std::string get_os_name() {
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

    std::pair<std::string, std::string> generate_form_data(std::vector<std::pair<std::ifstream, std::string>> &strm, nlohmann::json const &json_payload = {}) {
        std::stringstream data;

        std::string boundary{};
        for (int i = 0; i < 50; i++) {
            boundary += (rand() % 26) + 'A';
        }

        for (auto &each : strm) {
            std::string filename = "file";
            data << "--" << boundary << "\r\n";
            data << "Content-Disposition: form-data; name=\"file\"; filename=\""
                 << each.second << "\"\r\nContent-Type: application/octet-stream\r\n\r\n"
                 << read_entire_file(each.first) << "\r\n\r\n";
        }
        if (json_payload.size()) {
            data << "--" << boundary << "\r\nContent-Disposition: form-data; name=\"payload_json\"\r\n\r\n"
                 << json_payload.dump() << "\r\n";
        }
        data << "--" << boundary << "--";

        return { boundary, data.str() };
    }

}  // namespace discord
