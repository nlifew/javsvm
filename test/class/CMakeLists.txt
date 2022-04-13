cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        test_javah
        javah.cpp
        context.cpp
)

target_link_libraries(
        test_javah
        class
)