cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 23)

project(nebula-agent)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(Crow REQUIRED)

add_executable(nebula-agent src/agent.cpp)
target_link_libraries(nebula-agent PRIVATE Boost::boost fmt Crow::Crow)
target_include_directories(nebula-agent PRIVATE ${CMAKE_SOURCE_DIR}/include)
