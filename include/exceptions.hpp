#pragma once
#include "exception"

namespace discord {
    class ImproperToken : public std::exception {
        /**
         * @brief Exception that is thrown when an improper token is passed.
         */
        const char* what() const throw();
    };
} // namespace discord