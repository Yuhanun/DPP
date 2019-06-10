cmake_minimum_required(VERSION 3.14)
project(DPP)

find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)
find_package(Boost COMPONENTS system date_time REQUIRED)
find_package(Threads)

include_directories(${OpenSSL_INCLUDE_DIRS})
set(OPENSSL_USE_STATIC_LIBS TRUE)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(discord_pp STATIC
            src/activity.cpp
            src/assets.cpp
            src/attachment.cpp
            src/audit.cpp
            src/bot.cpp
            src/channel.cpp
            src/color.cpp
            src/context.cpp
            src/embedbuilder.cpp
            src/emoji.cpp
            src/exceptions.cpp
            src/guild.cpp
            src/integration.cpp
            src/invite.cpp
            src/member.cpp
            src/message.cpp
            src/object.cpp
            src/permissions.cpp
            src/presence.cpp
            src/result.cpp
            src/role.cpp
            src/user.cpp
            src/webhook.cpp
            src/utils.cpp)

add_executable(bot
               main.cpp)

set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH "./")

if(MSVC)
  target_compile_options(discord_pp PRIVATE /W4 /WX)
else()
  target_compile_options(discord_pp PRIVATE -Wall -Wextra)
endif()

target_link_libraries(discord_pp PUBLIC Boost::system)
target_link_libraries(discord_pp PUBLIC ${OpenSSL_LIBRARIES})
target_link_libraries(discord_pp PUBLIC OpenSSL::SSL)
target_link_libraries(discord_pp PUBLIC OpenSSL::Crypto)
target_link_libraries(discord_pp PUBLIC Threads::Threads)
target_link_libraries(discord_pp PUBLIC Boost::date_time)
target_link_libraries(discord_pp PUBLIC cpprest)

if(MSVC)
  target_compile_options(bot PRIVATE /W4 /WX)
else()
  target_compile_options(bot PRIVATE -Wall -Wextra)
endif()

target_link_libraries(bot PUBLIC Boost::system)
target_link_libraries(bot PUBLIC ${OpenSSL_LIBRARIES})
target_link_libraries(bot PUBLIC OpenSSL::SSL)
target_link_libraries(bot PUBLIC OpenSSL::Crypto)
target_link_libraries(bot PUBLIC Threads::Threads)
target_link_libraries(bot PUBLIC Boost::date_time)
target_link_libraries(bot PUBLIC cpprest)
target_link_libraries(bot PUBLIC discord_pp)