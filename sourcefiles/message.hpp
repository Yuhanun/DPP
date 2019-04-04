#pragma once
#include <discord.hpp>


namespace discord {
    class Message : public discord::Object {
    public:
        bool tts;
        
    };
};