#include "exceptions.hpp"

const char *discord::ImproperToken::what() const throw() {
    /**
     * @brief Exception's throw() method.
     */
    return "Improper token has been passed";
}
