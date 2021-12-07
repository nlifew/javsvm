cmake_minimum_required(VERSION 3.20)

add_executable(
        arm64
        arm64.cpp
)

include_directories(../..)

target_link_libraries(
        arm64
        engine
)