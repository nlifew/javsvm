
#
# this file is generated by autogen.sh, do NOT modify it.
#
cmake_minimum_required(VERSION 3.4.1)

include_directories(..)
include_directories(out)

add_library(
        jre
        STATIC
        native.cpp
        java/io/FileDescriptor.cpp
        java/io/FileInputStream.cpp
        java/io/FileOutputStream.cpp
        java/lang/Object.cpp
        java/lang/Class.cpp
        java/lang/String.cpp
        java/lang/System.cpp
        java/lang/Float.cpp
        java/lang/Thread.cpp
        sun/reflect/Reflection.cpp
)
