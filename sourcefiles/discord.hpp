#pragma once
#include <string>
#include <iostream>
typedef uint64_t discord_id;

namespace discord {
    class Object {
    public:

        Object() = default;

        Object(discord_id id)
            : id{id}
        {}

        discord_id id;
        // some datetime creation time.


        bool operator==(const Object& other){
            return this->id == other.id;
        }

        bool operator==(const discord_id& other){
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other){
            return !(std::forward<T>(other) == this);
        }

        friend std::ostream& operator<<(std::ostream& stream, Object& o){
            stream << o.id;
            return stream;
        }
    };
};
