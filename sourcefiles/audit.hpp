#pragma once
#include "discord.hpp"

namespace discord {
	AuditLogs::AuditLogs(const nlohmann::json& j) {
		audit_log_entries.push_back({
			j["id"],
			
		});
	}
} // namespace discord