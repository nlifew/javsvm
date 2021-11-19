cmake_minimum_required(VERSION 3.4.1)

add_library(
        vm
        STATIC
        jmethod_area.cpp
        jstack.cpp
        jheap.cpp
        jenv.cpp
        jvm.cpp
)