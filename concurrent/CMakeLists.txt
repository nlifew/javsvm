cmake_minimum_required(VERSION 3.4.1)

add_library(
        concurrent
        STATIC
        recursive_lock.cpp
        atomic_lock.cpp
)