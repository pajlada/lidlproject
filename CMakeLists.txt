cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

project(lidlproject CXX)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED system)

add_executable(lidlproject src/main.cc)

target_link_libraries(lidlproject ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
