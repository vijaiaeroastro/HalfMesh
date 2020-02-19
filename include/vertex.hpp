#pragma once
#include <general.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    // A vertex structure
    struct Vertex
    {
        Vertex(double _x1, double _x2, double _x3):x1(_x1), x2(_x2), x3(_x3){};
        double x1, x2, x3;
        unsigned int id;
    };
}