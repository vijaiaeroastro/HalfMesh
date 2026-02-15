cmake_minimum_required(VERSION 3.15)
project(halfMesh VERSION 1.2.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build usage examples" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

set(HALFMESH_SOURCES
    source/mesh_core.cpp
    source/mesh_algorithms.cpp
    source/mesh_traversers.cpp
    source/mesh_geometry.cpp
    source/mesh_io.cpp
    source/mesh_validation.cpp
)

set(HALFMESH_HEADERS
    include/common.hpp
    include/stream_utilities.hpp
    include/connectivity.hpp
    include/vertex.hpp
    include/half_edge.hpp
    include/edge.hpp
    include/face.hpp
    include/triMesh.hpp
    include/halfMesh.hpp
)

add_library(halfMesh ${HALFMESH_SOURCES} ${HALFMESH_HEADERS})
add_library(halfMesh::halfMesh ALIAS halfMesh)

target_include_directories(halfMesh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/eigen-3.4.0>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/nlohmann-3.12.0>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
    target_compile_options(halfMesh PRIVATE /W4 /permissive-)
else()
    target_compile_options(halfMesh PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_EXAMPLES)
    add_executable(halfMesh_example examples/basic_usage.cpp)
    target_link_libraries(halfMesh_example PRIVATE halfMesh::halfMesh)
    add_executable(halfMesh_validation_example examples/validation_workflow.cpp)
    target_link_libraries(halfMesh_validation_example PRIVATE halfMesh::halfMesh)
    add_executable(halfMesh_edge_split_example examples/edge_split_workflow.cpp)
    target_link_libraries(halfMesh_edge_split_example PRIVATE halfMesh::halfMesh)
    add_executable(halfMesh_edge_collapse_example examples/edge_collapse_workflow.cpp)
    target_link_libraries(halfMesh_edge_collapse_example PRIVATE halfMesh::halfMesh)
    add_executable(halfMesh_edge_flip_example examples/edge_flip_workflow.cpp)
    target_link_libraries(halfMesh_edge_flip_example PRIVATE halfMesh::halfMesh)
endif()

if(BUILD_TESTS)
    enable_testing()

    add_executable(halfMesh_test_topology tests/test_topology_geometry.cpp)
    target_link_libraries(halfMesh_test_topology PRIVATE halfMesh::halfMesh)
    add_test(NAME halfMesh_test_topology COMMAND halfMesh_test_topology)

    add_executable(halfMesh_test_validation tests/test_validation_properties.cpp)
    target_link_libraries(halfMesh_test_validation PRIVATE halfMesh::halfMesh)
    add_test(NAME halfMesh_test_validation COMMAND halfMesh_test_validation)

    add_executable(halfMesh_test_mutation tests/test_mutation_regressions.cpp)
    target_link_libraries(halfMesh_test_mutation PRIVATE halfMesh::halfMesh)
    add_test(NAME halfMesh_test_mutation COMMAND halfMesh_test_mutation)
endif()

install(TARGETS halfMesh
    EXPORT halfMeshTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${HALFMESH_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT halfMeshTargets
    NAMESPACE halfMesh::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/halfMesh
)
