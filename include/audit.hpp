#pragma once
#include "events.hpp"
#include "nlohmann/json.hpp"
#include "user.hpp"

namespace discord {

    class Asset;
    class Webhook;
    class Channel;
    class PermissionOverwrites;

    struct AuditLogKeyChange {
        /**
         * @brief A class that holds all possible things that changed in an Auditlog
         */
        AuditLogKeyChange(const nlohmann::json&, snowflake);
        AuditLogKeyChange() = default;
        std::string name;
        discord::Asset icon;
        discord::Asset splash;
        discord::User owner;
        std::string region;
        std::shared_ptr<discord::Channel> afk_channel;
        int afk_timeout;
        int mfa_level;
        int verification_level;
        int explicit_content_filter;
        int default_message_notifications;
        std::string vanity_url_code;
        int prune_delete_days;
        bool widget_enabled;
        std::shared_ptr<discord::Channel> widget_channel;
        int position;
        std::string topic;
        int bitrate;
        bool nsfw;
        snowflake application_id;
        std::string permissions;
        std::vector<discord::PermissionOverwrites> permission_overwrites;
        int color;
        bool hoist;
        bool mentionable;
        int allow;
        int deny;
        std::string code;
        std::shared_ptr<discord::Channel> channel;
        int max_uses;
        int uses;
        int max_age;
        bool temporary;
        bool deaf;
        bool mute;
        std::string nick;
        discord::Asset avatar;
        snowflake id;
        std::string type;
    };

    class AuditLogEntry {
        /**
         * @brief An entry in AuditLogs
         */
    public:
        AuditLogEntry() = default;
        AuditLogEntry(nlohmann::json const&);

        snowflake target_id;
        std::vector<AuditLogKeyChange> changes;
        snowflake user_id;
        snowflake id;
        AuditLogEventType action_type;
        int count;
        int type;
        std::string reason;

        struct {
            std::string delete_member_days;
            std::string members_removed;
            snowflake channel_id;
            std::string count;
            snowflake id;
            std::string type;
            std::string role_name;
        } opts;
        std::vector<decltype(opts)> options;
    };

    struct AuditLogs {
        /**
         * @brief A struct that holds all the audit logs, their corresponding users and webhooks.
         */
        AuditLogs() = default;
        AuditLogs(const nlohmann::json&);
        std::vector<discord::Webhook> webhooks;
        std::vector<discord::User> users;
        std::vector<discord::AuditLogEntry> audit_log_entries;
    };
}  // namespace discord