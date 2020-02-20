#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <vector>
#include <limits>
#include <cstddef>

namespace HalfMesh
{
    struct Vertex;
    struct Face;
    struct Edge;
    struct HalfEdge;

    // Some standard null pointers
    std::nullptr_t NULL_VERTEX;
    std::nullptr_t NULL_FACE;
    std::nullptr_t NULL_EDGE;
    std::nullptr_t NULL_HALF_EDGE;
}