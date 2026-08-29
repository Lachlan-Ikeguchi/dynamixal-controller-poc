cmake_minimum_required(VERSION 3.10)
project(dynamixal-controller-poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dynamixel_sdk CONFIG REQUIRED)

add_executable(dynamixal-controller src/main.cpp)

target_link_libraries(dynamixal-controller PRIVATE dynamixel_sdk::dynamixel_sdk)
