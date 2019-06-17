#pragma once
#include "activity.hpp"

namespace discord {
    
    class Role;

    class Presence {
    public:
        Presence() = default;
        Presence(const nlohmann::json);
        void update(nlohmann::json const);

        std::vector<discord::Role> roles;
        discord::Activity game;
        std::string status;
        std::vector<discord::Activity> activities;
        struct {
            std::string desktop;
            std::string mobile;
            std::string web;
        } client_status;
    };
} // namespace discord