cmake_minimum_required(VERSION 3.20)

include_directories(../..)

add_executable(
        test_pool
        pool.cpp
)

add_executable(
        test_recursive_lock
        recursive_lock.cpp
)

target_link_libraries(
        test_recursive_lock
        concurrent
)

add_executable(
        test_concurrent_map
        concurrent_map.cpp
)

target_link_libraries(
        test_concurrent_map
        concurrent
)