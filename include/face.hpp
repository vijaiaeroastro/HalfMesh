#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>

namespace HalfMesh
{
    // A face structure
    struct Face
    {
        Face(Vertex *_v1, Vertex *_v2, Vertex *_v3):v1(_v1), v2(_v2), v3(_v3){};
        Vertex *v1, *v2, *v3;
        unsigned int face_handle;
        HalfEdge *one_half_edge = NULL;
    };

}