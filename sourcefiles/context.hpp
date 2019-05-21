#pragma once
#include <discord.hpp>

template <typename... Tys>
discord::Message discord::Context::send(Tys&&... args) const {
    return this->message.channel->send(std::forward<Tys>(args)...);
};