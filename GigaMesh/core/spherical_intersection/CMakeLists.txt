cmake_minimum_required (VERSION 3.10)

target_sources(gigameshCore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mesh_spherical.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/mesh_spherical.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph_geometry.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph_geometry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graph_visualization.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph_visualization.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/math3d/math3d.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm/sphere_volume_msii.cpp    ${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_volume_msii.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm/sphere_intersections_msii.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_intersections_msii.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm/sphere_surface_msii.cpp   ${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_surface_msii.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm/component_count.cpp	${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/component_count.h
)

target_include_directories(gigameshCore PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
								$<INSTALL_INTERFACE:include>
							PRIVATE
								${CMAKE_CURRENT_SOURCE_DIR}/src
								${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection)
