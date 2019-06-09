#pragma once
#include "exception"

namespace discord {
    class ImproperToken : public std::exception {
        const char* what() const throw();
    };
};  // namespace discord