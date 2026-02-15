#include "halfMesh.hpp"

#include <cassert>
#include <cmath>
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

void test_reject_invalid_face_insertions() {
    halfMesh::triMesh mesh;
    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto v4 = mesh.add_vertex(1.0, 1.0, 0.0);

    const auto f0 = mesh.add_face(v1, v2, v3);
    assert(f0 != nullptr);

    assert(mesh.add_face(v1, v1, v2) == nullptr);
    assert(mesh.add_face(v1, v2, v4) == nullptr);

    mesh.complete_mesh();
    assert(mesh.is_valid());
    assert(mesh.get_faces().size() == 1);
}

void test_connected_components_after_handle_gaps() {
    halfMesh::triMesh mesh;
    const auto v0 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v1 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto v3 = mesh.add_vertex(3.0, 0.0, 0.0);
    const auto v4 = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto v5 = mesh.add_vertex(3.0, 1.0, 0.0);

    mesh.add_face(v0, v1, v2);
    mesh.add_face(v3, v4, v5);
    mesh.complete_mesh();
    assert(mesh.num_connected_components() == 2);

    assert(mesh.delete_face(mesh.get_faces().at(1)));
    assert(mesh.remove_unreferenced_vertices() == 3);
    const auto a = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(10.0, 1.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    assert(mesh.num_connected_components() == 2);
}

void test_split_boundary_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(mesh.can_split(edge_ab));

    const auto m = mesh.split_edge(edge_ab);
    assert(m != nullptr);
    mesh.complete_mesh();

    assert(std::abs(m->get_x() - 1.0) < 1e-12);
    assert(std::abs(m->get_y() - 0.0) < 1e-12);
    assert(std::abs(m->get_z() - 0.0) < 1e-12);

    assert(mesh.get_faces().size() == 2);
    assert(mesh.get_edges().size() == 5);
    assert(mesh.get_vertices().size() == 4);
    assert(mesh.validate().ok);
}

void test_split_interior_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(mesh.can_split(edge_ab));

    const auto m = mesh.split_edge(edge_ab);
    assert(m != nullptr);
    mesh.complete_mesh();

    assert(mesh.get_faces().size() == 4);
    assert(mesh.get_edges().size() == 8);
    assert(mesh.get_vertices().size() == 5);
    assert(mesh.validate().ok);
}

void test_split_edge_fraction_parameter() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);

    const auto p = mesh.split_edge(edge_ab, 0.25);
    assert(p != nullptr);
    mesh.complete_mesh();

    assert(std::abs(p->get_x() - 1.0) < 1e-12);
    assert(std::abs(p->get_y() - 0.0) < 1e-12);
    assert(std::abs(p->get_z() - 0.0) < 1e-12);
    assert(mesh.validate().ok);
}

void test_split_edge_fraction_rejects_endpoints() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);

    assert(mesh.split_edge(edge_ab, 0.0) == nullptr);
    assert(mesh.split_edge(edge_ab, 1.0) == nullptr);
    assert(mesh.split_edge(edge_ab, -0.1) == nullptr);
    assert(mesh.split_edge(edge_ab, 1.1) == nullptr);

    mesh.complete_mesh();
    assert(mesh.get_vertices().size() == 3);
    assert(mesh.get_faces().size() == 1);
    assert(mesh.validate().ok);
}

void test_collapse_interior_edge_to_endpoint() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(mesh.can_collapse(edge_ab, a));
    assert(mesh.collapse_edge(edge_ab, a));

    mesh.complete_mesh();
    assert(mesh.get_vertices().size() == 0);
    assert(mesh.get_faces().size() == 0);
    assert(mesh.validate().ok);
}

void test_collapse_rejects_non_incident_target_vertex() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 10.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(!mesh.can_collapse(edge_ab, x));
    assert(!mesh.collapse_edge(edge_ab, x));
}

void test_collapse_rejects_duplicate_face_result() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(a, c, d) != nullptr);
    assert(mesh.add_face(b, d, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);

    // Collapsing b->a would map (b,c,d) -> (a,c,d), duplicating an existing face.
    assert(!mesh.can_collapse(edge_ab, a));
    assert(!mesh.collapse_edge(edge_ab, a));
}

void test_flip_interior_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(mesh.can_flip(edge_ab));
    assert(mesh.flip_edge(edge_ab));

    mesh.complete_mesh();
    assert(mesh.get_vertices().size() == 4);
    assert(mesh.get_faces().size() == 2);
    assert(find_edge_between(mesh, c, d) != nullptr);
    assert(find_edge_between(mesh, a, b) == nullptr);
    assert(mesh.validate().ok);
}

void test_flip_rejects_boundary_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(!mesh.can_flip(edge_ab));
    assert(!mesh.flip_edge(edge_ab));
}

void test_flip_rejects_duplicate_face_result() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    assert(mesh.add_face(c, d, a) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);

    // Flipping AB would create triangle (c,d,a), which already exists.
    assert(!mesh.can_flip(edge_ab));
    assert(!mesh.flip_edge(edge_ab));
}
} // namespace

int main() {
    test_reject_invalid_face_insertions();
    test_connected_components_after_handle_gaps();
    test_split_boundary_edge();
    test_split_interior_edge();
    test_split_edge_fraction_parameter();
    test_split_edge_fraction_rejects_endpoints();
    test_collapse_interior_edge_to_endpoint();
    test_collapse_rejects_non_incident_target_vertex();
    test_collapse_rejects_duplicate_face_result();
    test_flip_interior_edge();
    test_flip_rejects_boundary_edge();
    test_flip_rejects_duplicate_face_result();
    std::cout << "halfMesh mutation/regression tests passed\n";
    return 0;
}
