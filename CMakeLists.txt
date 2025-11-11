cmake_minimum_required(VERSION 3.20)

project(kalshi_mm
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE)
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
endif()

# This warning is just noise about Boost; silence it if you want
cmake_policy(SET CMP0167 OLD)

find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(OpenSSL REQUIRED)

message(STATUS "Botan library: ${BOTAN_LIBRARY}")
message(STATUS "Botan include: ${BOTAN_INCLUDE_DIR}")

add_executable(kalshi_mm
    main.cpp
    order_book.cpp
    infra/engine.cpp
    protocols/kalshi/kalshi_ws_adapter.cpp
    protocols/kalshi/kalshi_auth.cpp
    protocols/kalshi/kalshi_order_book.cpp
    protocols/kalshi/kalshi_order_book_manager.cpp
    strategy/kalshi_mm.cpp
    utils/rsa_pss.cpp
)

target_include_directories(kalshi_mm PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(kalshi_mm PRIVATE
    Boost::boost
    nlohmann_json::nlohmann_json
    ixwebsocket::ixwebsocket
    OpenSSL::SSL OpenSSL::Crypto
)

# add_executable(test_kalshi_ws
#     test.cpp
#     protocols/kalshi/kalshi_auth.cpp
#     utils/rsa_pss.cpp
# )
#
# target_include_directories(test_kalshi_ws PRIVATE
#     "${PROJECT_SOURCE_DIR}"
# )
#
# target_link_libraries(test_kalshi_ws PRIVATE
#     Boost::boost               # if your test uses Boost, otherwise remove
#     nlohmann_json::nlohmann_json
#     ixwebsocket::ixwebsocket
#     OpenSSL::SSL
#     OpenSSL::Crypto
# )
