cmake_minimum_required (VERSION 3.10)

project (Input_Parser VERSION 1.0.0 DESCRIPTION "A simple command line argument parser library")
add_library(input_parser STATIC src/input_parser.cpp)
target_compile_features(input_parser PUBLIC cxx_std_17)
target_include_directories(input_parser PRIVATE include)
