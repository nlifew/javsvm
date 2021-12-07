cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        arm64
        arm64.cpp
)

target_link_libraries(
        arm64
        engine
)

add_executable(
        eng
        engine.cpp
)

target_link_libraries(
        eng
        classloader
        engine
        object
        vm
        class
        concurrent
        utils
)