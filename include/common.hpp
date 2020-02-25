#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <vector>
#include <limits>
#include <cstddef>

namespace HalfMesh {
    // Forward declaration of all classes
    class Vertex;

    class Face;

    class Edge;

    class HalfEdge;

    class Mesh;

    // Some standard null pointers
    std::nullptr_t NULL_VERTEX;
    std::nullptr_t NULL_FACE;
    std::nullptr_t NULL_EDGE;
    std::nullptr_t NULL_HALF_EDGE;
    std::nullptr_t NULL_POINTER;

    // Enum's for Mesh IO
    enum MESH_TYPE {
        GMSH,
        OBJ
    };

    // Enum for mesh identities
    enum MESH_ENTITY_TYPE {
        M_VERTEX,
        M_EDGE,
        M_FACE,
        M_HALF_EDGE
    };

    // Property addition and deletion status
    enum PROPERTY_STATUS {
        PROPERTY_ADDED,
        PROPERTY_EXISTS,
        PROPERTY_DELETED,
        PROPERTY_COULD_NOT_BE_DELETED,
        PROPERTY_DOES_NOT_EXIST,
        PROPERTY_COULD_NOT_BE_ADDED
    };
}