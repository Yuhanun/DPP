#pragma once
#include "discord.hpp"
#include "utility.hpp"

namespace discord {

	AuditLogEntry::AuditLogEntry(nlohmann::json const& j)
			: target_id{ to_sf(get_value(j, "target_id", 0)) },
			  user_id{ to_sf(get_value(j, "user_id", 0)) },
			  id{ to_sf(get_value(j, "id", 0)) },
			  channel_id{ to_sf(get_value(j, "channel_id", 0)) },
			  overwritten_id{ to_sf(get_value(j, "overwritten_id", 0)) },
			  count{ get_value(j, "count", 0) },
			  role_name{ get_value(j, "role_name", "") },
			  reason{ get_value(j, "reason", "") },
			  members_removed{ get_value(j, "members_removed", "") },
			  delete_member_days{ get_value(j, "delete_member_days", "") },
			  type{ get_value(j, "type", object_type{}) },
			  action_type{ get_value(j, "action_type", AuditLogEventType{}) } {
			if (!j["changes"].empty())
				for (const auto& elem : j["changes"])
					changes.emplace_back(elem);
	}

    AuditLogs::AuditLogs(const nlohmann::json& j) {
        audit_log_entries.emplace_back(j);
    }

    AuditLogKeyChange::AuditLogKeyChange(const nlohmann::json& j)
        : name{ get_value(j, "name", "") },
          icon_hash{ get_value(j, "icon_hash", "") },
          splash_hash{ get_value(j, "splash_hash", "") },
          owner_id{ to_sf(get_value(j, "owner_id", 0)) },
          region{ get_value(j, "region", "") },
          afk_channel_id{ to_sf(get_value(j, "afk_channel_id", 0)) },
          afk_timeout{ get_value(j, "afk_timeout", 0) },
          mfa_level{ get_value(j, "mfa_level", 0) },
          verification_level{ get_value(j, "verification_level", 0) },
          explicit_content_filter{ get_value(j, "verification_level", 0) },
          default_message_notifications{ get_value(j, "default_message_notifications", 0) },
          vanity_url_code{ get_value(j, "vanity_url_code", "") },
          prune_delete_days{ get_value(j, "prune_delete_days", 0) },
          widget_enabled{ get_value(j, "widget_enabled", false) },
          widget_channel_id{ to_sf(get_value(j, "widget_channel_id", 0)) },
          position{ get_value(j, "position", 0) },
          topic{ get_value(j, "topic", "") },
          bitrate{ get_value(j, "bitrate", 0) },
          nsfw{ get_value(j, "nsfw", false) },
          application_id{ to_sf(get_value(j, "application_id", 0)) },
          permission{ get_value(j, "permission", 0) },
          color{ get_value(j, "color", 0) },
          hoist{ get_value(j, "hoist", false) },
          mentionable{ get_value(j, "mentionable", false) },
          allow{ get_value(j, "allow", 0) },
          deny{ get_value(j, "deny", 0) },
          code{ get_value(j, "code", "") },
          channel_id{ to_sf(get_value(j, "channel_id", "")) },
          max_uses{ get_value(j, "max_uses", 0) },
          uses{ get_value(j, "uses", 0) },
          max_age{ get_value(j, "max_age", 0) },
          temporary{ get_value(j, "temporary", false) },
          deaf{ get_value(j, "deaf", false) },
          mute{ get_value(j, "mute", false) },
          nick{ get_value(j, "nick", "") },
          avatar_hash{ get_value(j, "avatar_hash", "") },
          id{ to_sf(get_value(j, "id", 0)) },
          type{ get_value(j, "type", "") } {
    }
}  // namespace discord
