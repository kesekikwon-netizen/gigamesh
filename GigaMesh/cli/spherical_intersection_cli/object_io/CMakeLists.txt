cmake_minimum_required (VERSION 3.10)

project (Object_IO VERSION 1.0.0 DESCRIPTION "A simple mesh input output library")
add_library(object_io STATIC src/obj.cpp src/ply.cpp)
target_compile_features(object_io PUBLIC cxx_std_17)
target_include_directories(object_io PRIVATE include)

