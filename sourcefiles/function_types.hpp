#pragma once
#include <tuple>
#include <vector>
#include <functional>
#include <iostream>


template <typename...Funcs>
struct Events
{
    std::tuple<std::vector<std::function<Funcs>>...> tuple;

    template <size_t index, typename Func>
    void add(Func&& func)
    {
        std::get<index>(tuple).push_back(std::forward<Func>(func));
    }
    template <size_t index, typename...Args>
    void call(Args&&...args)
    {
        for(auto& func : std::get<index>(tuple))
        {
            func(std::forward<Args>(args)...);
        }
    }
};