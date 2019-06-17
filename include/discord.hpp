#pragma once
#include <nlohmann/json.hpp>

#include <cpprest/http_client.h>


namespace discord {

    class Bot;
    class User;
    class Role;
    class Color;
    class Emoji;
    class Guild;
    class Asset;
    class Member;
    class Invite;
    class Webhook;
    class Channel;
    class Message;
    class Activity;
    class Presence;
    class Attachment;
    class Integration;
    class EmbedBuilder;
    class AuditLogEntry;
    class PermissionOverwrite;
    class PermissionOverwrites;

    template <typename T>
    struct Result;
    struct Context;
    struct AuditLogs;
    struct AuditLogKeyChange;
}  // namespace discord