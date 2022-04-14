cmake_minimum_required(VERSION 3.20)
project(javsvm)


message("Build type: ${CMAKE_BUILD_TYPE}")

# define platform information here.
# enum {
#   platform_macos_arm64
#   // todo
# }
set(PLATFORM_OS_ARCH "platform_macos_arm64")

set(CMAKE_CXX_STANDARD 17)
set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Dplatform_os_arch=${PLATFORM_OS_ARCH} -Werror=format -fdata-sections -ffunction-sections")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(C_FLAGS "${C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections")
else ()
    set(C_FLAGS "${C_FLAGS} -O0 -g")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS}")

enable_language(C ASM)

add_subdirectory(class)
add_subdirectory(concurrent)
add_subdirectory(object)
add_subdirectory(vm)
add_subdirectory(classloader)
add_subdirectory(engine)
add_subdirectory(utils)
add_subdirectory(dll)
add_subdirectory(jni)
add_subdirectory(gc)
add_subdirectory(jre)

add_library(
        runtime
        STATIC
        runtime.cpp
)



target_link_libraries(
        runtime
        jre
        classloader
        engine
        object
        gc
        vm
        jni
        class
        dllloader
        concurrent
        utils
)

# 测试相关的
add_subdirectory(test)

add_executable(
        main
        main.cpp
)

target_link_libraries(
        main
        runtime
)