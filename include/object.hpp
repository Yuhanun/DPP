#pragma once
#include <cstdint>
#include "discord.hpp"

namespace discord {
    
    typedef int64_t snowflake;

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