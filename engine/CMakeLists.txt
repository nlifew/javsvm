cmake_minimum_required(VERSION 3.4.1)

add_library(
        engine
        STATIC
        run_java.cpp
        run_jni.cpp
        throw.cpp
        arm64.asm
)