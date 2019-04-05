#pragma once
#include <string>

#include "nlohmann/json.hpp"

namespace discord{
    using namespace nlohmann;

    template <typename T>
    T get_value(json& j, const char* s, T default_value){
        return j[s].empty() ? default_value : j[s].get<T>();
    }

    inline std::string get_value(json& j, const char* s, const char* default_value){
        return j[s].empty() ? default_value : j[s].get<std::string>();
    }
}