cmake_minimum_required (VERSION 3.10)

project (Timer VERSION 1.0.0 DESCRIPTION "A simple timer library")
add_library(timer STATIC src/timer.cpp)
target_compile_features(timer PUBLIC cxx_std_17)
target_include_directories(timer PRIVATE include)
