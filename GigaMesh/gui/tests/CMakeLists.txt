# DongArch3D - Test Automation (Phase 8)
# Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute

cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find Qt5
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL Test REQUIRED)

# Test executable
add_executable(test_dongarch
    test_dongarch.cpp
)

# Include directories
target_include_directories(test_dongarch PRIVATE
    ${CMAKE_SOURCE_DIR}/gui/src
    ${CMAKE_SOURCE_DIR}/core/mesh
)

# Link libraries
target_link_libraries(test_dongarch
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Test
    GigaMesh::libgigamesh
)

# C++20 standard
set_target_properties(test_dongarch PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add test to CTest
add_test(NAME DongArch3D_Tests COMMAND test_dongarch)

# Test timeout (10 minutes)
set_tests_properties(DongArch3D_Tests PROPERTIES TIMEOUT 600)

# Optional: Valgrind memory check (Linux only)
if(UNIX AND NOT APPLE)
    find_program(VALGRIND_EXECUTABLE valgrind)
    if(VALGRIND_EXECUTABLE)
        add_test(NAME DongArch3D_MemoryCheck
                 COMMAND ${VALGRIND_EXECUTABLE}
                         --leak-check=full
                         --error-exitcode=1
                         $<TARGET_FILE:test_dongarch>)
        set_tests_properties(DongArch3D_MemoryCheck PROPERTIES TIMEOUT 1200)
    endif()
endif()

# Test output
set_tests_properties(DongArch3D_Tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"  # Headless testing
)

message(STATUS "DongArch3D tests configured")
