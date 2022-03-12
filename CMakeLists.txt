cmake_minimum_required(VERSION 3.0)

project(quicklist-client)

add_executable(quicklist-client main.cpp)

install(TARGETS quicklist-client RUNTIME DESTINATION bin)
