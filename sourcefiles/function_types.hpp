#pragma once
#include <tuple>
#include <vector>
#include <functional>
#include <iostream>

namespace discord{
    class Guild;
    class Channel;
    class Member;
    class Message;
};


template <typename...Funcs>
struct Events
{
    std::tuple<std::vector<std::function<Funcs>>...> tuple;

    template <size_t index, typename Func>
    void add(Func&& func)
    {
        std::get<index>(tuple).push_back(std::forward<Func>(func));
    }
    template <size_t index, typename...Args>
    void call(Args&&...args)
    {
        for(auto& func : std::get<index>(tuple))
        {
            func(std::forward<Args>(args)...);
        }
    }
};

typedef Events<
    void(), // READY 
    void(discord::Message, std::exception, std::string), // ERROR
    void(discord::Guild), // GUILD_STATUS
    void(discord::Guild), // GUILD_CREATE
    void(discord::Channel), // CHANNEL_CREATE
    void(discord::Channel), // VOICE_CHANNEL_SELECT
    void(discord::Member, discord::Channel), // VOICE_STATE_CREATE
    void(discord::Member, discord::Channel), // VOICE_STATE_UPDATE
    void(discord::Member, discord::Channel), // VOICE_STATE_DELETE
    void(), // VOICE_SETTINGS_UPDATE
    void(discord::Channel), // VOICE_CONNECTION_STATUS
    void(discord::Member), // SPEAKING_START
    void(discord::Member), // SPEAKING_STOP
    void(discord::Message), // MESSAGE_CREATE
    void(discord::Message), // MESSAGE_UPDATE
    void(discord::Message), // MESSAGE_DELETE
    void(discord::Message), // NOTIFICATION_CREATE
    void(), // CAPTURE_SHORTCUT_CHANGE
    void(), // ACTIVITY_JOIN
    void(), // ACTIVITY_SPECTATE
    void()> function_handler; // ACTIVITY_JOIN_REQUEST