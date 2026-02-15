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

    const auto edit = mesh.split_edge(edge_ab);
    assert(edit.ok);
    assert(edit.created_vertices.size() == 1);
    const auto m = mesh.get_vertex(edit.created_vertices.front());
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

    const auto edit = mesh.split_edge(edge_ab);
    assert(edit.ok);
    assert(edit.created_vertices.size() == 1);
    const auto m = mesh.get_vertex(edit.created_vertices.front());
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

    const auto edit = mesh.split_edge(edge_ab, 0.25);
    assert(edit.ok);
    assert(edit.created_vertices.size() == 1);
    const auto p = mesh.get_vertex(edit.created_vertices.front());
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

    assert(!mesh.split_edge(edge_ab, 0.0).ok);
    assert(!mesh.split_edge(edge_ab, 1.0).ok);
    assert(!mesh.split_edge(edge_ab, -0.1).ok);
    assert(!mesh.split_edge(edge_ab, 1.1).ok);

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
    assert(mesh.collapse_edge(edge_ab, a).ok);

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
    assert(!mesh.collapse_edge(edge_ab, x).ok);
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
    assert(!mesh.collapse_edge(edge_ab, a).ok);
}

void test_collapse_accepts_valid_boundary_link_condition() {
    halfMesh::triMesh mesh;
    const auto u = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    assert(mesh.add_face(u, v, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    assert(edge_uv != nullptr);
    assert(mesh.can_collapse(edge_uv, u));
}

void test_collapse_rejects_interior_link_condition_violation() {
    halfMesh::triMesh mesh;
    const auto u = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto d = mesh.add_vertex(1.0, 1.0, 0.0);
    const auto w = mesh.add_vertex(0.5, 0.5, 1.0);
    const auto a = mesh.add_vertex(-1.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);

    assert(mesh.add_face(u, v, c) != nullptr);
    assert(mesh.add_face(v, u, d) != nullptr);
    assert(mesh.add_face(u, w, a) != nullptr); // creates edge u-w
    assert(mesh.add_face(v, b, w) != nullptr); // creates edge v-w
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    assert(edge_uv != nullptr);
    assert(!mesh.can_collapse(edge_uv, u));
}

void test_collapse_rejects_boundary_link_condition_violation() {
    halfMesh::triMesh mesh;
    const auto u = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto w = mesh.add_vertex(0.5, 0.5, 1.0);
    const auto a = mesh.add_vertex(-1.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);

    assert(mesh.add_face(u, v, c) != nullptr); // boundary edge uv
    assert(mesh.add_face(u, w, a) != nullptr); // creates edge u-w
    assert(mesh.add_face(v, b, w) != nullptr); // creates edge v-w
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    assert(edge_uv != nullptr);
    assert(!mesh.can_collapse(edge_uv, u));
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
    assert(mesh.flip_edge(edge_ab).ok);

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
    assert(!mesh.flip_edge(edge_ab).ok);
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
    assert(!mesh.flip_edge(edge_ab).ok);
}

void test_split_edit_result_and_property_propagation() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto f = mesh.add_face(a, b, c);
    assert(f != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_vertex_property("temperature", 0.0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("temperature", a->get_handle(), 0.0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("temperature", b->get_handle(), 8.0) == halfMesh::PropertyStatus::Added);
    assert(mesh.add_face_property("region", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_face_property("region", f->get_handle(), 7) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    assert(mesh.add_edge_property("weight", 0.0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_edge_property("weight", edge_ab->get_handle(), 3.5) == halfMesh::PropertyStatus::Added);

    const auto edit = mesh.split_edge(edge_ab, 0.25);
    assert(edit.ok);
    assert(edit.error.empty());
    assert(edit.created_vertices.size() == 1);
    assert(!edit.created_edges.empty());
    assert(edit.removed_edges.size() == 1);
    assert(edit.removed_faces.size() == 1);
    assert(edit.created_faces.size() == 2);

    mesh.complete_mesh();
    const auto mid = mesh.get_vertex(edit.created_vertices.front());
    assert(mid != nullptr);

    double mid_temperature = -1.0;
    assert(mesh.try_get_vertex_property("temperature", mid->get_handle(), mid_temperature));
    assert(std::abs(mid_temperature - 2.0) < 1e-12);

    for (const auto new_face_handle: edit.created_faces) {
        int region = -1;
        assert(mesh.try_get_face_property("region", new_face_handle, region));
        assert(region == 7);
    }
}

void test_collapse_edit_result_has_vertex_remap_and_properties() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_vertex_property("label", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("label", a->get_handle(), 10) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("label", b->get_handle(), 20) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    const auto edit = mesh.collapse_edge(edge_ab, a);
    assert(edit.ok);
    assert(!edit.vertex_handle_remap.empty());
    assert(edit.removed_vertices.size() == 1);
    assert(edit.removed_vertices.front() == b->get_handle());

    mesh.complete_mesh();
    const auto remapped_a = edit.vertex_handle_remap.at(a->get_handle());
    int label = -1;
    assert(mesh.try_get_vertex_property("label", remapped_a, label));
    assert(label == 10);
}

void test_flip_edit_result_and_face_property_remap() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto f0 = mesh.add_face(a, b, c);
    const auto f1 = mesh.add_face(b, a, d);
    assert(f0 != nullptr);
    assert(f1 != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_face_property("id", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_face_property("id", f0->get_handle(), 101) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_face_property("id", f1->get_handle(), 202) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    const auto edit = mesh.flip_edge(edge_ab);
    assert(edit.ok);
    assert(edit.removed_faces.size() == 2);
    assert(edit.created_faces.size() == 2);

    mesh.complete_mesh();
    assert(find_edge_between(mesh, c, d) != nullptr);

    const auto remapped_f0 = edit.face_handle_remap.at(f0->get_handle());
    const auto remapped_f1 = edit.face_handle_remap.at(f1->get_handle());
    int id0 = -1;
    int id1 = -1;
    assert(mesh.try_get_face_property("id", remapped_f0, id0));
    assert(mesh.try_get_face_property("id", remapped_f1, id1));
    assert(id0 == 101);
    assert(id1 == 202);
}

void test_flip_keeps_unrelated_handles_stable() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    assert(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("marker", x->get_handle(), 99) == halfMesh::PropertyStatus::Added);

    const auto old_x_handle = x->get_handle();
    const auto old_extra_face_handle = mesh.get_faces().back()->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    const auto edit = mesh.flip_edge(edge_ab);
    assert(edit.ok);
    mesh.complete_mesh();

    // Unrelated component should keep handles and properties unchanged.
    assert(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    assert(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    assert(marker == 99);
    assert(mesh.get_face(old_extra_face_handle) != nullptr);
}

void test_split_keeps_unrelated_handles_stable() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("marker", x->get_handle(), 77) == halfMesh::PropertyStatus::Added);

    const auto old_x_handle = x->get_handle();
    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    const auto edit = mesh.split_edge(edge_ab, 0.5);
    assert(edit.ok);
    mesh.complete_mesh();

    // Unrelated component remains stable.
    assert(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    assert(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    assert(marker == 77);
    assert(edit.vertex_handle_remap.count(old_x_handle) == 1);
    assert(edit.vertex_handle_remap.at(old_x_handle) == old_x_handle);
}

void test_collapse_keeps_unrelated_handles_stable() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    assert(mesh.add_face(a, b, c) != nullptr);
    assert(mesh.add_face(b, a, d) != nullptr);
    assert(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    assert(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    assert(mesh.try_set_vertex_property("marker", x->get_handle(), 55) == halfMesh::PropertyStatus::Added);
    const auto old_x_handle = x->get_handle();
    const auto old_extra_face_handle = mesh.get_faces().back()->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    assert(edge_ab != nullptr);
    const auto edit = mesh.collapse_edge(edge_ab, a);
    assert(edit.ok);
    mesh.complete_mesh();

    assert(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    assert(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    assert(marker == 55);
    assert(mesh.get_face(old_extra_face_handle) != nullptr);
    assert(edit.vertex_handle_remap.count(old_x_handle) == 1);
    assert(edit.vertex_handle_remap.at(old_x_handle) == old_x_handle);
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
    test_collapse_accepts_valid_boundary_link_condition();
    test_collapse_rejects_interior_link_condition_violation();
    test_collapse_rejects_boundary_link_condition_violation();
    test_flip_interior_edge();
    test_flip_rejects_boundary_edge();
    test_flip_rejects_duplicate_face_result();
    test_split_edit_result_and_property_propagation();
    test_collapse_edit_result_has_vertex_remap_and_properties();
    test_flip_edit_result_and_face_property_remap();
    test_flip_keeps_unrelated_handles_stable();
    test_split_keeps_unrelated_handles_stable();
    test_collapse_keeps_unrelated_handles_stable();
    std::cout << "halfMesh mutation/regression tests passed\n";
    return 0;
}
