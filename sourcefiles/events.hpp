#pragma once 
#include <unordered_map>

const inline std::unordered_map<std::string, int> event_m{
    {"READY", 0},
    {"ERROR", 1},
    {"GUILD_STATUS", 2},
    {"GUILD_CREATE", 3},
    {"CHANNEL_CREATE", 4},
    {"VOICE_CHANNEL_SELECT", 5},
    {"VOICE_STATE_CREATE", 6},
    {"VOICE_STATE_UPDATE", 7},
    {"VOICE_STATE_DELETE", 8},
    {"VOICE_SETTINGS_UPDATE", 9},
    {"VOICE_CONNECTION_STATUS", 10},
    {"SPEAKING_START", 11},
    {"SPEAKING_STOP", 12},
    {"MESSAGE_CREATE", 13},
    {"MESSAGE_UPDATE", 14},
    {"MESSAGE_DELETE", 15},
    {"NOTIFICATION_CREATE", 16},
    {"CAPTURE_SHORTCUT_CHANGE", 17},
    {"ACTIVITY_JOIN", 18},
    {"ACTIVITY_SPECTATE", 19},
    {"ACTIVITY_JOIN_REQUEST", 20},
};

constexpr int READY = 0;
constexpr int ERROR = 1;
constexpr int GUILD_STATUS = 2;
constexpr int GUILD_CREATE = 3;
constexpr int CHANNEL_CREATE = 4;
constexpr int VOICE_CHANNEL_SELECT = 5;
constexpr int VOICE_STATE_CREATE = 6;
constexpr int VOICE_STATE_UPDATE = 7;
constexpr int VOICE_STATE_DELETE = 8;
constexpr int VOICE_SETTINGS_UPDATE = 9;
constexpr int VOICE_CONNECTION_STATUS = 10;
constexpr int SPEAKING_START = 11;
constexpr int SPEAKING_STOP = 12;
constexpr int MESSAGE_CREATE = 13;
constexpr int MESSAGE_UPDATE = 14;
constexpr int MESSAGE_DELETE = 15;
constexpr int NOTIFICATION_CREATE = 16;
constexpr int CAPTURE_SHORTCUT_CHANGE = 17;
constexpr int ACTIVITY_JOIN = 18;
constexpr int ACTIVITY_SPECTATE = 19;
constexpr int ACTIVITY_JOIN_REQUEST = 20;

