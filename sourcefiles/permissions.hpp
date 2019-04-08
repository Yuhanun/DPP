#pragma once
#include "discord.hpp"
#include "experimental/vector"
#include "utility.hpp"

discord::PermissionOverwrite::PermissionOverwrite(std::string const& name, bool value)
    : name{name}, value{value}
{}

discord::PermissionOverwrites::PermissionOverwrites(const int value)
    : value{value}
{
    for (auto const& each : discord::permission_overwrites){
        if ((each.second & value) == each.second){
            add_overwrite(PermissionOverwrite(each.first, true));
        } else {
            add_overwrite(PermissionOverwrite(each.first, false));
        }     
    }
}

discord::PermissionOverwrites::PermissionOverwrites(const int& value, discord_id owner, std::string const& type_input)
    : value{value}, object_id{owner}
{
    type = type_input == "role";
    for (auto const& each : discord::permission_overwrites){
        if ((each.second & value) == each.second){
            add_overwrite(PermissionOverwrite(each.first, true));
        } else {
            add_overwrite(PermissionOverwrite(each.first, false));
        }     
    }
}


discord::PermissionOverwrites::PermissionOverwrites(std::vector<PermissionOverwrite> const& ows){
    overwrites = ows;
    calculate_value();
}

discord::PermissionOverwrites& discord::PermissionOverwrites::remove_overwrite(std::string const& name){
    overwrites.erase(std::remove_if(overwrites.begin() , overwrites.end(), [&](const PermissionOverwrite& o){
        return o.name == name;
    }));
    calculate_value();
    return *this;
}

discord::PermissionOverwrites& discord::PermissionOverwrites::add_overwrite(PermissionOverwrite const& overwrite){
    overwrites.push_back(overwrite);
    calculate_value();
    if ((value & 0x8) == 0x8){
        overwrites.clear();
        for (auto const& each : discord::permission_overwrites){
            overwrites.push_back(discord::PermissionOverwrite(each.first, true));    
        }
    }
    return *this;
}

discord::PermissionOverwrite discord::PermissionOverwrites::get_overwrite(std::string const& name) const {
    for (auto const& each : overwrites){
        if (each.name != name){
            continue;
        }
        return each;
    }
    return PermissionOverwrite("INVALID_PERMISSION", false);
}

int discord::PermissionOverwrites::get_value() const {
    return value;
}

void discord::PermissionOverwrites::calculate_value(){
    value = 0;
    for (auto const& perm : discord::permission_overwrites){
        for (auto const& each : overwrites){
            if (each.value == true){
                value |= perm.second;
            }
        }
    }
}

std::vector<discord::PermissionOverwrite>::const_iterator discord::PermissionOverwrites::begin() const noexcept {
    return overwrites.begin();
}

std::vector<discord::PermissionOverwrite>::const_iterator discord::PermissionOverwrites::end() const noexcept {
    return overwrites.end();
}