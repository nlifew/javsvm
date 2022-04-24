cmake_minimum_required(VERSION 3.20)

add_subdirectory(engine)
add_subdirectory(concurrent)
add_subdirectory(object)
add_subdirectory(class)
add_subdirectory(utils)

add_executable(
        test_main
        main.cpp
)

target_link_libraries(
        test_main
        runtime
)