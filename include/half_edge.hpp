#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    // A Half edge structure
    class HalfEdge
    {
        public:
            HalfEdge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
            Vertex *v1, *v2;
            unsigned int half_edge_handle;
            unsigned int parent_edge_handle;
            unsigned int parent_face_handle;
            unsigned int opposing_half_edge = std::numeric_limits<unsigned int>::max();
            unsigned int next_half_edge;
            bool boundary_half_edge;
    };
}
