#include "object.hpp"

namespace discord {

    Object::Object(snowflake id)
        : id{ id } {
    }

    bool Object::operator==(const Object& other) const {
        return this->id == other.id;
    }

    bool Object::operator==(const snowflake& other) const {
        return this->id == other;
    }

    Object::operator snowflake() const {
        return id;
    }

    std::ostream& operator<<(std::ostream& stream, Object const& o) {
        return stream << "<discord::Object id=" << o.id << " at " << &o << ">";
    }
};  // namespace discord