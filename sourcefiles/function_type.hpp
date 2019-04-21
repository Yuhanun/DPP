#pragma once
#include <functional>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <vector>

template <typename... Funcs>
struct Events {
    std::tuple<std::vector<std::function<Funcs>>...> tuple;

    template <size_t index, typename Func>
    void add(Func &&func) {
        std::get<index>(tuple).push_back(std::forward<Func>(func));
    }
    template <size_t index, typename... Args>
    void call(bool ready, Args &&... args) {
        if (!ready) {
            return;
        }
        for (auto &func : std::get<index>(tuple)) {
            func(std::forward<Args>(args)...);
        }
    }
};