#pragma once
#include "object.hpp"
#include "discord.hpp"

namespace discord {
    class Message : public Object {
    public:
        Message() = default;
        Message(snowflake);
        Message(nlohmann::json const);
        Message& update(nlohmann::json const);  // TODO

        pplx::task<discord::Message> edit(std::string);
        pplx::task<discord::Message> edit(EmbedBuilder, std::string = "");
        pplx::task<void> pin();
        pplx::task<void> unpin();
        pplx::task<void> remove();

        pplx::task<void> add_reaction(discord::Emoji const&);
        pplx::task<void> remove_own_reaction(discord::Emoji const&);
        pplx::task<void> remove_reaction(discord::User const&, discord::Emoji const&);
        pplx::task<void> remove_all_reactions();
        pplx::task<std::vector<discord::User>> get_reactions(discord::Emoji const&, snowflake = 0, snowflake = 0, int = 0);

    public:
        int type;

        bool tts;
        bool sent;
        bool pinned;
        bool mention_everyone;

        snowflake id;

        std::string content;
        datetime timestamp{ boost::local_time::not_a_date_time };
        datetime edited_timestamp{ boost::local_time::not_a_date_time };

        std::shared_ptr<discord::Member> author;
        std::shared_ptr<discord::Channel> channel;

        std::vector<std::shared_ptr<discord::Member>> mentions;
        std::vector<discord::Role> mentioned_roles;
        std::vector<discord::Attachment> attachments;
        std::vector<discord::EmbedBuilder> embeds;

    private:
        std::string token;
    };
} // namespace discord