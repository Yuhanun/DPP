# DPP
A C++ Library interfacing Discord API, or should i say... C++ Discord API Wrapper.

## Dependencies
- [JSON](https://github.com/nlohmann/json)
- [Boost::asio](https://www.boost.org/)
- [Boost::system](https://www.boost.org/)
- [Boost::date_time](https://www.boost.org/)
- [OpenSSL](https://www.openssl.org/)
- [cpprest](https://github.com/microsoft/cpprestsdk)
- [websocketpp](https://github.com/zaphoyd/websocketpp)

## Installation ##
- vcpkg install nlohmann-json
- vcpkg install cpprestsdk
- vcpkg install boost

- git clone https://www.github.com/yuhanun/DPP
- cd DPP
- mkdir build && cd build
- vcpkg integrate install
- cmake .. -DCMAKE_TOOLCHAIN_FILE=/use/what/integrate/install/gives
- make


## Example
```cpp
#include "attachment.hpp"
#include "bot.hpp"
#include "context.hpp"
#include "embedbuilder.hpp"
#include "events.hpp"
#include "member.hpp"
#include "role.hpp"
#include "user.hpp"
#include "utils.hpp"
#include "emoji.hpp"
#include "message.hpp"

int main() {
    discord::Bot bot{ "tokenhere", ">" };

    bot.register_callback<discord::events::ready>([&bot]() {
        std::cout << "Ready!" << std::endl
                  << "Logged in as: " << bot.username << "#" << bot.discriminator
                  << std::endl
                  << "ID: " << bot.id << std::endl
                  << "-----------------------------" << std::endl;
    });

    bot.register_command("ping", "Ping?", {}, [](discord::Context ctx) {
        ctx.channel->send("pong!")
            .wait();
    }, {});

    return bot.run();
}
```

### License
See [LICENSE](LICENSE)
