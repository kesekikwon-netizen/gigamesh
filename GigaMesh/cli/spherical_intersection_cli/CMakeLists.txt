cmake_minimum_required (VERSION 3.10)
project(Spherical_intersection_standalones VERSION 1.0.0 DESCRIPTION "mesh sphere intersection standalone tools")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(gigamesh-featurevectors-sl src/standalone.cpp)
add_executable(gigamesh-sphere-profiles src/profiles.cpp)
add_subdirectory(input_parser)
add_subdirectory(timer)
add_subdirectory(object_io)

target_link_libraries(gigamesh-featurevectors-sl PRIVATE Threads::Threads gigameshCore input_parser timer object_io)
target_link_libraries(gigamesh-sphere-profiles PRIVATE Threads::Threads gigameshCore input_parser timer object_io)
target_include_directories(gigamesh-featurevectors-sl PRIVATE input_parser/include object_io/include timer/include)
target_include_directories(gigamesh-sphere-profiles PRIVATE input_parser/include object_io/include timer/include)

install(TARGETS gigamesh-featurevectors-sl
                gigamesh-sphere-profiles
        DESTINATION bin)
