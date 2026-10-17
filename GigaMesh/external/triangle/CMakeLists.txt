cmake_minimum_required(VERSION 3.10)

project(triangle LANGUAGES CXX C)

add_library(triangle STATIC
    src/triangle.c
    src/triangleTriangleIntersection.cpp
    include/triangle/triangleTriangleIntersection.h
    include/triangle/triangle.h)

target_include_directories(triangle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(triangle PRIVATE -DNO_TIMER)
