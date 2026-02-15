#include "halfMesh.hpp"

#include <iostream>

int main() {
    halfMesh::triMesh mesh;

    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);

    mesh.add_face(v1, v2, v3);
    mesh.complete_mesh();

    std::cout << "Vertices: " << mesh.get_vertices().size() << '\n';
    std::cout << "Faces: " << mesh.get_faces().size() << '\n';
    std::cout << "Surface area: " << mesh.surface_area() << '\n';

    mesh.save("triangle.stl");
    return 0;
}
