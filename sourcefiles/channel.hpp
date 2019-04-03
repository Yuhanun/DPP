#include <discord.hpp>

namespace discord {
    class Channel : public discord::Object{
        Channel(discord_id id) : discord::Object(id){
            
        }
    };
};