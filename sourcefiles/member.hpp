#include <user.hpp>

namespace discord {
    class Member : public discord::User {
    public:
        Member(discord_id id) : discord::User(id) {

        }

        Member() = default;
    };
};

