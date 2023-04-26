cmake_minimum_required(VERSION 3.24)
project(chatRoom)

set(CMAKE_CXX_STANDARD 17)
set(Boost_INCLUDE_DIR "/usr/local/Boost/boost_1_82_0")

find_package(Boost 1.66.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(chatRoom main.cpp websocket_session.hpp websocket_listener.hpp)
target_link_libraries(chatRoom ${Boost_LIBRARIES})