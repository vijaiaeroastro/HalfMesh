#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <half_edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    // An edge structure
    struct Edge
    {
        Edge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
        Vertex *v1, *v2;
        unsigned int edge_handle;
        HalfEdge *one_half_edge = NULL;

    };
}
