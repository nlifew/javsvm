cmake_minimum_required(VERSION 3.4.1)

add_library(
        object
        STATIC
        jmethod.cpp
        jfield.cpp
        jobject.cpp
        jclass.cpp
#        jstring.cpp
        jarray.cpp
)
