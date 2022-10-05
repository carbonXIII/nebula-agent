cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 23)

project(nebula-agent)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(Crow REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(nebula-agent src/agent.cpp)
target_link_libraries(nebula-agent PRIVATE Boost::boost fmt::fmt Crow::Crow yaml-cpp cpr::cpr)
target_include_directories(nebula-agent PRIVATE ${CMAKE_SOURCE_DIR}/include)
