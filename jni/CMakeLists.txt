cmake_minimum_required(VERSION 3.4.1)

add_library(
        jni
        STATIC
        JNIEnv.cpp
        JavaVM.cpp
)