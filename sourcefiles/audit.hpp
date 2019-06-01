#pragma once
#include "discord.hpp"

namespace discord {
    AuditLogEntry::AuditLogEntry(const snowflake& user_id, const int& action_type, const snowflake& before, const int& init) {
        this->id = user_id;
        this->action_type = discord::AuditLogEvents(action_type);
    }
    AuditLogs::AuditLogs(const nlohmann::json& j) {
        AuditLogEntry temp;
        audit_log_entries.push_back({ j["id"],
                                      j["type"],
                                      j["before"],
                                      j["limit"] });
    }
}  // namespace discord