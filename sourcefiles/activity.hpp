#pragma once
#include "discord.hpp"
#include "utility.hpp"


discord::Activity::Activity(std::string const& name, presence::activity const& type, std::string const& status, bool const& afk, std::string const& url)
    : name{ name }, type{ type }, url{ url }, status{ status }, afk{ afk } {
}

nlohmann::json discord::Activity::to_json() const {
    auto payload = nlohmann::json({ { "game", { { "name", name }, { "type", static_cast<int>(type) } } },
                                    { "status", status },
                                    { "afk", afk },
                                    { "since", nullptr } });
    if (type == presence::activity::streaming) {
        payload["game"]["url"] = url;
    }
    return payload;
}