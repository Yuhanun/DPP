#pragma once
#include "object.hpp"
#include "discord.hpp"
#include "permissions.hpp"

namespace discord {
    class Channel : public Object {
    public:
        Channel() = default;
        Channel(snowflake id);

        Channel(nlohmann::json const, snowflake);

        Channel& update(nlohmann::json const);

        pplx::task<discord::Message> send(std::string const&, std::vector<File> const& = {}, bool = false) const;
        pplx::task<discord::Message> send(EmbedBuilder const&, std::vector<File> const& = {}, bool = false, std::string const& = "") const;
        pplx::task<discord::Message> get_message(snowflake);
        pplx::task<discord::Invite> create_invite(int = 86400, int = 0, bool = false, bool = false) const;
        pplx::task<std::vector<discord::Invite>> get_invites();
        pplx::task<std::vector<discord::Message>> get_pins();
        pplx::task<void> remove_permissions(discord::Object const&);
        pplx::task<void> typing();

        pplx::task<std::vector<discord::Message>> get_messages(int);

        pplx::task<void> bulk_delete(std::vector<discord::Message>&);

        pplx::task<void> edit(nlohmann::json&);
        pplx::task<void> remove();

        // TODO: avatar
        pplx::task<discord::Webhook> create_webhook(std::string const&);
        pplx::task<std::vector<discord::Webhook>> get_webhooks();

        pplx::task<void> add_group_dm_recipient(discord::User const&, std::string const&, std::string const&);
        pplx::task<void> remove_group_dm_recipient(discord::User const&);

        pplx::task<void> edit_position(int);

    public:
        int type;
        int bitrate;
        int position;
        int user_limit;
        int rate_limit_per_user;

        std::shared_ptr<discord::Channel> parent;
        snowflake last_message_id;

        std::string name;
        std::string topic;

        std::shared_ptr<discord::Guild> guild;
        std::vector<std::shared_ptr<discord::User>> recipients;
        std::vector<discord::PermissionOverwrites> overwrites;
    };
};  // namespace discord