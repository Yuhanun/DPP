#pragma once
#include <discord.hpp>

namespace discord {
    class Channel : public discord::Object{
    public:
        Channel() : discord::Object(){}
        Channel(discord_id id) : discord::Object(id){
            
        }
    };
};