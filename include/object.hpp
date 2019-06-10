#pragma once
#include "discord.hpp"

namespace discord {
    class Object {
    public:
        Object() = default;

        Object(snowflake id);

        snowflake id;

        bool operator==(const Object& other) const;
        bool operator==(const snowflake& other) const;

        explicit operator snowflake() const;
        friend std::ostream& operator<<(std::ostream& stream, Object const& o);

    };
} // namespace discord