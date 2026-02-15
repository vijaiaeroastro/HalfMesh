#include "halfMesh.hpp"

#include <iostream>

namespace {
halfMesh::edgePtr find_edge_between(const halfMesh::triMesh &mesh,
                                    const halfMesh::vertexPtr &a,
                                    const halfMesh::vertexPtr &b) {
    for (const auto &e: mesh.get_edges()) {
        const auto v1 = e->get_vertex_one();
        const auto v2 = e->get_vertex_two();
        if (!v1 || !v2) {
            continue;
        }
        const bool direct = v1->get_handle() == a->get_handle() && v2->get_handle() == b->get_handle();
        const bool reverse = v1->get_handle() == b->get_handle() && v2->get_handle() == a->get_handle();
        if (direct || reverse) {
            return e;
        }
    }
    return nullptr;
}
} // namespace

int main() {
    halfMesh::triMesh mesh;

    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);

    mesh.add_face(a, b, c);
    mesh.add_face(b, a, d);
    mesh.complete_mesh();

    std::cout << "Before collapse: V=" << mesh.get_vertices().size()
              << " E=" << mesh.get_edges().size()
              << " F=" << mesh.get_faces().size() << '\n';

    const auto edge_ab = find_edge_between(mesh, a, b);
    if (!edge_ab || !mesh.can_collapse(edge_ab, a)) {
        std::cerr << "Edge AB cannot be collapsed onto A\n";
        return 1;
    }

    if (!mesh.collapse_edge(edge_ab, a)) {
        std::cerr << "Edge collapse failed\n";
        return 1;
    }
    mesh.complete_mesh();

    std::cout << "After collapse: V=" << mesh.get_vertices().size()
              << " E=" << mesh.get_edges().size()
              << " F=" << mesh.get_faces().size() << '\n';
    std::cout << "Mesh valid: " << (mesh.validate().ok ? "yes" : "no") << '\n';
    return 0;
}
