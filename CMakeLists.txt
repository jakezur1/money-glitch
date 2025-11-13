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

# Silence Boost CMP0167 warning
cmake_policy(SET CMP0167 OLD)

find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED) # <-- for gtest / gmock

message(STATUS "OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "GTest include dir: ${GTEST_INCLUDE_DIRS}")

# -----------------------
# Core library
# -----------------------
add_library(kalshi_core
    order_book.cpp
    infra/engine.cpp
    protocols/kalshi/kalshi_ws_adapter.cpp
    protocols/kalshi/kalshi_auth.cpp
    protocols/kalshi/kalshi_order_book.cpp
    protocols/kalshi/kalshi_order_book_manager.cpp
    strategy/kalshi_mm.cpp
    utils/rsa_pss.cpp
    strategy/positions/position_manager.cpp
    strategy/positions/ticker_position_ledger.cpp
)

target_include_directories(kalshi_core PUBLIC
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(kalshi_core PUBLIC
    Boost::boost
    nlohmann_json::nlohmann_json
    ixwebsocket::ixwebsocket
    OpenSSL::SSL
    OpenSSL::Crypto
)

# -----------------------
# Main executable
# -----------------------
add_executable(kalshi_mm
    main.cpp
)

target_link_libraries(kalshi_mm PRIVATE kalshi_core)

# -----------------------
# Tests
# -----------------------
enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*_test.cpp)

foreach(test_file IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE
        kalshi_core
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
