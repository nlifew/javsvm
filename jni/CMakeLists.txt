cmake_minimum_required(VERSION 3.4.1)

add_library(
        jni
        STATIC
        access_array.cpp
        access_field.cpp
        call_method.cpp
        jni_env.cpp
        java_vm.cpp
        nio.cpp
        reference.cpp
        reflection.cpp
        string_utf.cpp
)