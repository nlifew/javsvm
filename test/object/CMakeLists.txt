cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        test_jobject
        jobject.cpp
)

target_link_libraries(
        test_jobject
        object
        engine
        classloader
        vm
        dllloader
        class
        concurrent
        utils
)