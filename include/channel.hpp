#pragma once
#include "object.hpp"
#include "permissions.hpp"

namespace discord {
    class Channel : public Object {
        /**
     * @brief a discord::Channel object, to which messages can be sent
     * 
     * ```cpp
     *      discord::utils::get(bot.channels, [](auto& chan){ return chan->id == 570591769302007838; }).send("Hello!").wait();
     * 
     *      // or
     * 
     *      discord::Channel{ 570591769302007838 }.send("Hello!").wait();
     * ```
     */
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

        pplx::task<discord::Channel> edit(nlohmann::json&);
        pplx::task<void> remove();

        // TODO: avatar
        pplx::task<discord::Webhook> create_webhook(std::string const&);
        pplx::task<std::vector<discord::Webhook>> get_webhooks();

        pplx::task<void> add_group_dm_recipient(discord::User const&, std::string const&, std::string const&);
        pplx::task<void> remove_group_dm_recipient(discord::User const&);

        pplx::task<void> edit_position(int);

    public:
        int type; /**< Type of the channel */
        int bitrate; /**< Bitrate of the channel */
        int position; /**< Position of the channel */
        int user_limit; /**< User limit of the (Voice)channel */
        int rate_limit_per_user; /**< Ratelimits for the (Voice)channel */

        std::shared_ptr<discord::Channel> parent; /**< Parent (Category)channel, can be default constructed */
        snowflake last_message_id; /**< Snowflake, id, of the last message sent */

        std::string name; /**< Name of the channel */
        std::string topic; /**< Topic of the channel */

        std::shared_ptr<discord::Guild> guild; /**< Pointer to the guild this belongs to, can be default constructed */
        std::vector<std::shared_ptr<discord::User>> recipients; /**< Recipients of this channel, can be default constructed */
        std::vector<discord::PermissionOverwrites> overwrites; /**< PermissionOverwrites for this channel */
    };
} // namespace discord