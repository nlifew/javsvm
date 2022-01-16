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
        test_run_java
        run_java.cpp
)

add_executable(
        test_jnilib
        jnilib.cpp
)


target_link_libraries(
        test_run_java
        runtime
)
