cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        test_utils
        utils.cpp
)

target_link_libraries(
        test_utils
        utils
        iconv
)