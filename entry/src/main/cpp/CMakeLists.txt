# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(Termony)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${NATIVERENDER_ROOT_PATH}
                              ${NATIVERENDER_ROOT_PATH}/include)

find_library(
    EGL-lib
    EGL
)

find_library(
    GLES-lib
    GLESv3
)

add_library(entry SHARED napi_init.cpp)
target_link_libraries(entry PUBLIC libace_napi.z.so ${EGL-lib} ${GLES-lib} libnative_window.so)