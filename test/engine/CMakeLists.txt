cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        test_arm64
        arm64.cpp
)

target_link_libraries(
        test_arm64
        engine
)

add_executable(
        test_engine
        engine.cpp
)

target_link_libraries(
        test_engine
        classloader
        engine
        object
        vm
        class
        dllloader
        concurrent
        utils
)