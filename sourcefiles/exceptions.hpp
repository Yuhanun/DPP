#pragma once

#include "discord.hpp"

const char *discord::ImproperToken::what() const throw() {
    return "Improper token has been passed";
}
