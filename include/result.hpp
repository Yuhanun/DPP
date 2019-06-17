#pragma once
#include <cpprest/http_client.h>
#include <variant>
#include "discord.hpp"


namespace discord {

    using namespace web::http;

    struct Err {
        std::string error;
        std::string url;
        nlohmann::json data;
        std::unordered_map<std::string, std::string> headers;

        http_response response;

        Err() = default;
        Err(std::string const& err, nlohmann::json const& data, std::unordered_map<std::string, std::string> const& headers, http_response& resp);
    };

    template <typename T>
    struct Ok {
        Ok() = default;

        Ok(T val)
            : value{ std::move(val) } {
        }

        T value;
    };

    template <typename T>
    struct Result {
    private:
        bool _is_ok;
        std::variant<Err, T> var_obj;

    public:
        Result() = default;

        Result(Ok<T> const& value)
            : _is_ok{ true }, var_obj{ value.value } {
        }

        Result(Err const& error)
            : _is_ok{ false }, var_obj{ error } {
        }

        ~Result(){};

        Result& operator=(Result&& other) = default;
        Result& operator=(Result const& other) = default;

        Result(Result&& other) {
            _is_ok = other.is_ok();
            if (is_ok()) {
                var_obj = std::move(other.value);
            } else {
                var_obj = std::move(other.err);
            }
        }

        Result(Result const& other)
            : var_obj{ other.var_obj } {
            _is_ok = other.is_ok();
        }

        auto is_ok() const noexcept -> bool {
            return _is_ok;
        }

        auto is_err() const noexcept -> bool {
            return !is_ok();
        }

        auto unwrap() const -> T const& {
            if (is_ok()) {
                return std::get<T>(var_obj);
            }
            throw std::runtime_error{ std::to_string(std::get<Err>(var_obj).response.status_code()) + std::string(": ") + std::get<Err>(var_obj).error };
        }

        auto unwrap_or(T const& value) const noexcept -> T const& {
            if (is_ok()) {
                return std::get<T>(var_obj);
            }
            return value;
        }

        auto expect(std::string const& msg) const -> T const& {
            if (!is_ok()) {
                throw std::runtime_error{ msg };
            }
            return std::get<T>(var_obj);
        }

        auto unwrap_err() const -> Err {
            if (is_ok()) {
                throw std::runtime_error{
                    "Error was unwrapped when contained value was \"Ok\""
                };
            }
            return std::get<Err>(var_obj);
        }

        auto expect_err(std::string const& msg) const -> Err const& {
            if (!is_err()) {
                throw std::runtime_error{ msg };
            }
            return std::get<Err>(var_obj);
        }
    };

    template <typename T>
    Result<T> OK(T const& val) {
        return { Ok{ val } };
    }

    template <typename T>
    Result<T> ERR(std::string const& err, nlohmann::json const& data, std::unordered_map<std::string, std::string> const& headers, http_response& resp) {
        return { Err{ err, data, headers, resp } };
    }

}  // namespace discord