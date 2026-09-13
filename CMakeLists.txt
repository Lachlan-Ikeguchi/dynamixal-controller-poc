cmake_minimum_required(VERSION 3.10)
project(dynamixal-controller-poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dynamixel_sdk CONFIG REQUIRED)

add_executable(dynamixal-controller src/main.cpp src/dynamixel_controller.cpp)

target_include_directories(dynamixal-controller PRIVATE include)
target_link_libraries(dynamixal-controller PRIVATE dynamixel_sdk::dynamixel_sdk)

install(TARGETS dynamixal-controller RUNTIME DESTINATION bin)
