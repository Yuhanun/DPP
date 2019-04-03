#include <string>

typedef uint64_t discord_id;


namespace discord {
    class Object{
    public:
        Object(discord_id id)
            : id{id}
        {

        }

        discord_id id;
        // some datetime creation time.

        bool operator==(const Object& other){
            return this->id == other.id;
        }

        bool operator==(discord_id other){
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other){
            return !(std::forward<T>(other) == this);
        }
    };
};