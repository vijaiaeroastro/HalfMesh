cmake_minimum_required(VERSION 3.15)
project(HalfMesh)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(HDRS include/common.hpp
         include/edge.hpp
         include/face.hpp
         include/general.hpp
         include/half_edge.hpp
         include/mesh.hpp
         include/vertex.hpp)

add_executable(HalfMesh main.cpp ${HDRS})