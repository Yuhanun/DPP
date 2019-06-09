#include "result.hpp"
#include "nlohmann/json.hpp"
#include "cpprest/http_client.h"

namespace discord {
    Err::Err(std::string const& err, nlohmann::json const& data, std::unordered_map<std::string, std::string> const& headers, http_response& resp)
        : error{ err }, data{ data }, headers{ headers }, response{ resp } {
    }
};  // namespace discord