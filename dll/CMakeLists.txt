cmake_minimum_required(VERSION 3.4.1)


find_library(
        dl-lib
        dl
)


add_library(
        dllloader
        STATIC
        dll_loader.cpp
)

target_link_libraries(
        dllloader
        ${dl-lib}
)