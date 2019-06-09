#pragma once
#include <functional>
#include <future>
#include <iostream>
#include <tuple>
#include <vector>

template <typename... Funcs>
struct Events {
    std::tuple<std::vector<std::function<Funcs>>...> tuple;

    template <size_t index, typename Func>
    void add(Func&& func) {
        std::get<index>(tuple).push_back(std::forward<Func>(func));
    }

    template <size_t index, typename... Args>
    void call(std::vector<std::future<void>>& future_lst, bool ready, Args&&... args) {
        if (!ready) {
            return;
        }
        for (auto& func : std::get<index>(tuple)) {
            future_lst.push_back(std::async(std::launch::async, func, std::forward<Args>(args)...));
        }
    }
};