#pragma once
#include <user.hpp>

namespace discord {
    class Member : public discord::User {
    public:
        Member() = default;
        
        Member(discord_id id) : discord::User(id) {

        }

    };
};

