cmake_minimum_required(VERSION 3.10)

project(libpsalm LANGUAGES CXX)

add_library(psalm STATIC
	src/libpsalm.cpp		include/libpsalm/libpsalm.h
	src/directed_edge.cpp	src/directed_edge.h
	src/edge.cpp			src/edge.h
	src/face.cpp			src/face.h
	src/mesh.cpp			src/mesh.h
	src/v3ctor.cpp			src/v3ctor.h
	src/vertex.cpp			src/vertex.h
	src/FairingAlgorithms/FairingAlgorithm.cpp	src/FairingAlgorithms/FairingAlgorithm.h
	src/SegmentationAlgorithms/PlanarSegmentation.cpp src/SegmentationAlgorithms/PlanarSegmentation.h
	src/SegmentationAlgorithms/SegmentationAlgorithm.cpp src/SegmentationAlgorithms/SegmentationAlgorithm.h
	src/SubdivisionAlgorithms/BsplineSubdivisionAlgorithm.cpp src/SubdivisionAlgorithms/BsplineSubdivisionAlgorithm.h
	src/SubdivisionAlgorithms/CatmullClark.cpp src/SubdivisionAlgorithms/CatmullClark.h
	src/SubdivisionAlgorithms/DooSabin.cpp	src/SubdivisionAlgorithms/DooSabin.h
	src/SubdivisionAlgorithms/Liepa.cpp		src/SubdivisionAlgorithms/Liepa.h
	src/SubdivisionAlgorithms/Loop.cpp		src/SubdivisionAlgorithms/Loop.h
	src/SubdivisionAlgorithms/SubdivisionAlgorithm.cpp	src/SubdivisionAlgorithms/SubdivisionAlgorithm.h
	src/TriangulationAlgorithms/MinimumWeightTriangulation.cpp	src/TriangulationAlgorithms/MinimumWeightTriangulation.h
	src/TriangulationAlgorithms/TriangulationAlgorithm.cpp		src/TriangulationAlgorithms/TriangulationAlgorithm.h
)

target_compile_features(psalm PRIVATE cxx_std_17)

target_include_directories(psalm PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
