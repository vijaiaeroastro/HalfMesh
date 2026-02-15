#include "halfMesh.hpp"

#include <iostream>

int main() {
    halfMesh::triMesh mesh;

    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto v4 = mesh.add_vertex(1.0, 1.0, 0.0);

    const auto f0 = mesh.add_face(v1, v2, v3);
    const auto f_bad = mesh.add_face(v1, v2, v4); // rejected: reuses directed half-edge (v1->v2)

    std::cout << "Created base face: " << (f0 ? "yes" : "no") << '\n';
    std::cout << "Created invalid face: " << (f_bad ? "yes" : "no") << '\n';

    auto report_before = mesh.validate();
    std::cout << "Valid before complete_mesh(): " << (report_before.ok ? "yes" : "no") << '\n';

    mesh.complete_mesh();

    auto report_after = mesh.validate();
    std::cout << "Valid after complete_mesh(): " << (report_after.ok ? "yes" : "no") << '\n';
    std::cout << "Faces: " << mesh.get_faces().size() << '\n';
    std::cout << "Edges: " << mesh.get_edges().size() << '\n';
    std::cout << "Half-edges: " << mesh.get_half_edges().size() << '\n';

    return 0;
}
