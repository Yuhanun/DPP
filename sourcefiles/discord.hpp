#include <string>

namespace discord{
    class Object{
    public:
        Object(uint64_t id)
            : id{id}
        {

        }

        uint64_t id;
        // some datetime creation time.

        bool operator==(const discord::Object& other){
            return this->id == other.id;
        }

        bool operator==(uint64_t other){
            return this->id == other;
        }
        template <typename T>
        bool operator!=(T&& other){
            return !(std::forward<T>(other) == this);
        }
    };

    class Color {
        int r;
        int g;
        int b;
    };

    class User : public discord::Object {
    public:
        User(uint64_t id) : discord::Object(id) {

        }
        bool bot;
        std::string name;
        std::string discriminator;
        std::string avatar;
        std::string avatar_url;
        discord::Color color;
        std::string display_name;
        std::string mention;
        
    };

    class Member : public discord::User {
        // discord::Channel;
    };

    class Message : public discord::Object {
        bool tts;
        
    };
};