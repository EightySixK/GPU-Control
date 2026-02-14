cmake_minimum_required(VERSION 3.16)
project(gpu-control LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(gpu-control gpu-control.cpp)
target_link_libraries(gpu-control Qt5::Widgets)

install(TARGETS gpu-control DESTINATION bin)
