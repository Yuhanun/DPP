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
            std::cout << response_stream.str() << std::endl;
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
}