cmake_minimum_required(VERSION 3.10)

set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable(gigameshCore_tests testsMain.cpp
                                  mesh_tests.cpp
                                  meshIO_tests.cpp
                                  icoSphereTests.cpp)
target_link_libraries(gigameshCore_tests PRIVATE Catch gigameshCore)

add_test(NAME GigameshCoreTests COMMAND gigameshCore_tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
