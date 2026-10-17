cmake_minimum_required(VERSION 3.10)

#-- INDIVIDUAL TOOLS--
add_executable(gigamesh-tolegacy gigamesh-tolegacy.cpp)
target_link_libraries(gigamesh-tolegacy PRIVATE gigameshCore)

add_executable(gigamesh-togltf gigamesh-togltf.cpp)
target_link_libraries(gigamesh-togltf PRIVATE gigameshCore)

add_executable(gigamesh-gnsphere gigamesh-gnsphere.cpp)
target_link_libraries(gigamesh-gnsphere PRIVATE gigameshCore)

add_executable(gigamesh-meshsplit gigamesh-meshsplit.cpp)
target_link_libraries(gigamesh-meshsplit PRIVATE gigameshCore)

add_executable(gigamesh-clean gigamesh-clean.cpp)
target_link_libraries(gigamesh-clean PRIVATE gigameshCore psalm)
target_compile_options(gigamesh-clean PRIVATE -DLIBPSALM)

add_executable(gigamesh-info gigamesh-info.cpp)
target_link_libraries(gigamesh-info PRIVATE gigameshCore)

add_executable( gigamesh-featurevectors meshgeneratefeaturevectors25d.cpp)
target_link_libraries( gigamesh-featurevectors PRIVATE gigameshCore)
target_include_directories( gigamesh-featurevectors PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/voxel
														  ${CMAKE_CURRENT_SOURCE_DIR}/mesh
														  ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gigamesh-borders gigamesh-borders.cpp)
target_link_libraries(gigamesh-borders PRIVATE gigameshCore)

install(TARGETS gigamesh-tolegacy
                gigamesh-clean
                gigamesh-info
                gigamesh-featurevectors
                gigamesh-borders
                gigamesh-gnsphere
                gigamesh-togltf
        DESTINATION bin)


add_subdirectory(spherical_intersection_cli)


#add_executable(meshgeneratefeaturevectors25d mesh/meshgeneratefeaturevectors25d.cpp ${SOURCES_COMMON} ${HEADERS_COMMON})
#target_link_libraries(meshgeneratefeaturevectors25d)
#target_compile_options(meshgeneratefeaturevectors25d PUBLIC -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")
#target_include_directories( meshgeneratefeaturevectors25d PRIVATE ${CMAKE_SOURCE_DIR}/voxel
#															  ${CMAKE_SOURCE_DIR}/mesh
#															  ${CMAKE_SOURCE_DIR})
#target_compile_features(meshgeneratefeaturevectors25d PUBLIC cxx_std_17)

#if(UNIX)
#	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE stdc++fs pthread)
#elseif(MSVC)
#    target_compile_options(meshgeneratefeaturevectors25d PRIVATE /permissive- -DNO_TIMER -DNOMINMAX)
#    target_link_libraries(meshgeneratefeaturevectors25d PRIVATE Advapi32 Ws2_32)
#elseif(MINGW)
#	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE ws2_32)
#endif()


