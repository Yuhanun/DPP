#include "exceptions.hpp"

const char *discord::ImproperToken::what() const throw() {
    return "Improper token has been passed";
}
