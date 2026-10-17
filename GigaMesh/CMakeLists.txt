cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

# CMAKEFile for building gigamesh

# note: building DongArch3D requires a compiler which supports C++20 features. So please
# make sure that your compiler has the following version or newer:
# gcc        : 10
# clang      : 10
# msvc stl   : 19.20 (Visual Studio 2019 16.0+)
# apple clang: 13.0.0
#
# on ubuntu for example, you have the following options to set the gcc10 compiler, assuming it is in usr/bin/:
# 1. Running cmake with:
# cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-10 -DCMAKE_CC_COMPILER=/usr/bin/gcc-10
#
# 2. Export CC and CXX before calling cmake:
# export CC=/usr/bin/gcc-10
# export CXX=/usr/bin/g++-10
#
# 3. Set g++-10 as alternative permanently:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 90 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
#


project(DongArch3D VERSION 1.0.0 LANGUAGES CXX C
			DESCRIPTION "동국문화재연구원 전용 실측 프로그램 - Dongguk Archaeological 3D Measurement System")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# C++20 features used in DongArch3D v4:
# - std::span (mesh data passing)
# - std::ranges (filtering, transformation)
# - Concepts (template constraints)
# - Designated initializers (struct init)
# - constexpr extensions (compile-time math)

# FetchContent for automatic dependency management (GLM)
include(FetchContent)

# GLM (OpenGL Mathematics) - used for vector/matrix operations
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1  # Updated for CMake 3.5+ compatibility (Windows build fix)
)

# Populate glm and patch its CMakeLists.txt for CMake 3.5+ compatibility
# (glm 1.0.1 requires CMake 2.6, but Windows CMake 4.1 needs 3.5+)
FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)

    # Patch glm's CMakeLists.txt to require CMake 3.5 instead of 2.6
    file(READ "${glm_SOURCE_DIR}/CMakeLists.txt" GLM_CMAKE_CONTENT)
    string(REPLACE
        "cmake_minimum_required(VERSION 2.6)"
        "cmake_minimum_required(VERSION 3.5)"
        GLM_CMAKE_CONTENT
        "${GLM_CMAKE_CONTENT}"
    )
    file(WRITE "${glm_SOURCE_DIR}/CMakeLists.txt" "${GLM_CMAKE_CONTENT}")

    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()



option(BUILD_CLI "Build the GigaMesh-CLI tools" ON)
option(BUILD_GUI "Build GigaMesh-GUI" ON)
option(ENABLE_TESTS "Build tests" ON)

#--------- get git version and other useful information ---------
string(TIMESTAMP COMP_DATE "%d.%b.%y")
execute_process(COMMAND git rev-parse HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE COMP_GITHEAD)
string(STRIP "${COMP_GITHEAD}" COMP_GITHEAD)

execute_process(COMMAND git log -1 --format=%ci
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE VERSION_PACKAGE)
#set(COMP_EDIT IWR\ Open\ Source)
set(COMP_EDIT 동국문화재연구원)
string(STRIP "${VERSION_PACKAGE}" VERSION_PACKAGE)
string(SUBSTRING "${VERSION_PACKAGE}" 2 9 VERSION_PACKAGE)
string(REPLACE "-" "" VERSION_PACKAGE "${VERSION_PACKAGE}")
string(STRIP "${VERSION_PACKAGE}" VERSION_PACKAGE)

if(UNIX OR MINGW)
	execute_process(COMMAND whoami OUTPUT_VARIABLE WHOAMIVAR)
	execute_process(COMMAND hostname -s OUTPUT_VARIABLE HOSTVAR)

	string (STRIP "${WHOAMIVAR}" WHOAMIVAR)
	string (STRIP "${HOSTVAR}" HOSTVAR)
	string(CONCAT COMP_USER ${WHOAMIVAR} @ ${HOSTVAR})
elseif(MSVC)
	execute_process(COMMAND whoami OUTPUT_VARIABLE WHOAMIVAR)
	string(STRIP "${WHOAMIVAR}" WHOAMIVAR)
	string(FIND "${WHOAMIVAR}" "\\" SLASHPOS REVERSE)

	string(SUBSTRING "${WHOAMIVAR}" 0 ${SLASHPOS} HOST_VAR)
	math(EXPR SLASHPOS "${SLASHPOS} + 1")
	string(SUBSTRING "${WHOAMIVAR}" "${SLASHPOS}" -1 USER_VAR)
	string(CONCAT COMP_USER ${USER_VAR} @ ${HOST_VAR})
endif()
#-------------------------------------------------------------

add_subdirectory(external)
add_subdirectory(core)

#disable CLI build by setting -DBUILD_CLI=OFF, or editing the variable in CMakeCache.txt
if(BUILD_CLI)
add_subdirectory(cli)
endif()

#disable GUI build by setting -DBUILD_GUI=OFF, or editing the variable in CMakeCache.txt
if(BUILD_GUI)
add_subdirectory(gui)
endif()

#disable test build by setting -DENABLE_TESTS=OFF, or editing the variable in CMakeCache.txt
if(ENABLE_TESTS)
	include(CTest)
	add_subdirectory(tests)
endif()

install(FILES	CHANGELOG
				README.md
				README_LIBPSALM.txt
				COPYING.txt
				LICENSE.txt
		DESTINATION share/doc/gigamesh)

#CPack, see: https://cmake.org/cmake/help/v3.10/module/CPack.html
include(InstallRequiredSystemLibraries)
#set variables before include(CPack) to override defaults of CPack
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_VERSION "${VERSION_PACKAGE}")
set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_STRIP_FILES "bin/gigamesh;bin/gigamesh-tolegacy;bin/gigamesh-togltf;bin/gigamesh-clean;bin/gigamesh-info;bin/gigamesh-featurevectors;bin/gigamesh-borders")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hubert Mara <hubert.mara@informatik.uni-halle.de>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libgcc1 (>= 1:3.0), qtbase5-dev(>= 5.5),libqt5core5a (>= 5), libqt5gui5 (>= 5) | libqt5gui5-gles (>= 5), libqt5opengl5 (>= 5) | libqt5opengl5-gles (>= 5), libstdc++6 (>= 5), inkscape(>=0.92)")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.gigamesh.eu")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")

#set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")

include(CPack)

