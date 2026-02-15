#include "halfMesh.hpp"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <unordered_set>

inline void hm_check(bool cond, const char *expr, const char *file, int line) {
    if (!cond) {
        std::cerr << "HM_CHECK failed: " << expr << " at " << file << ":" << line << "\n";
        std::abort();
    }
}
#define HM_CHECK(expr) hm_check((expr), #expr, __FILE__, __LINE__)

namespace {
std::unordered_set<unsigned> as_set(const std::vector<unsigned> &v) {
    return std::unordered_set<unsigned>(v.begin(), v.end());
}

bool are_disjoint(const std::vector<unsigned> &a, const std::vector<unsigned> &b) {
    const auto sb = as_set(b);
    for (const auto h: a) {
        if (sb.count(h)) {
            return false;
        }
    }
    return true;
}

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

void test_collapse_interior_edge_to_endpoint() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(mesh.can_collapse(edge_ab, a));
    HM_CHECK(mesh.collapse_edge(edge_ab, a).ok);

    mesh.complete_mesh();
    HM_CHECK(mesh.get_vertices().size() == 0);
    HM_CHECK(mesh.get_faces().size() == 0);
    HM_CHECK(mesh.validate().ok);
}

void test_collapse_rejects_non_incident_target_vertex() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 10.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(!mesh.can_collapse(edge_ab, x));
    HM_CHECK(!mesh.collapse_edge(edge_ab, x).ok);
}

void test_collapse_rejects_null_target() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    halfMesh::vertexPtr null_target;
    HM_CHECK(!mesh.can_collapse(edge_ab, null_target));
    HM_CHECK(!mesh.collapse_edge(edge_ab, null_target).ok);
}

void test_collapse_rejects_duplicate_face_result() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(a, c, d) != nullptr);
    HM_CHECK(mesh.add_face(b, d, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);

    HM_CHECK(!mesh.can_collapse(edge_ab, a));
    HM_CHECK(!mesh.collapse_edge(edge_ab, a).ok);
}

void test_collapse_accepts_valid_boundary_link_condition() {
    halfMesh::triMesh mesh;
    const auto u = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(u, v, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    HM_CHECK(edge_uv != nullptr);
    HM_CHECK(mesh.can_collapse(edge_uv, u));
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

    HM_CHECK(mesh.add_face(u, v, c) != nullptr);
    HM_CHECK(mesh.add_face(v, u, d) != nullptr);
    HM_CHECK(mesh.add_face(u, w, a) != nullptr);
    HM_CHECK(mesh.add_face(v, b, w) != nullptr);
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    HM_CHECK(edge_uv != nullptr);
    HM_CHECK(!mesh.can_collapse(edge_uv, u));
}

void test_collapse_rejects_boundary_link_condition_violation() {
    halfMesh::triMesh mesh;
    const auto u = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto w = mesh.add_vertex(0.5, 0.5, 1.0);
    const auto a = mesh.add_vertex(-1.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);

    HM_CHECK(mesh.add_face(u, v, c) != nullptr);
    HM_CHECK(mesh.add_face(u, w, a) != nullptr);
    HM_CHECK(mesh.add_face(v, b, w) != nullptr);
    mesh.complete_mesh();

    const auto edge_uv = find_edge_between(mesh, u, v);
    HM_CHECK(edge_uv != nullptr);
    HM_CHECK(!mesh.can_collapse(edge_uv, u));
}

void test_collapse_edit_result_has_vertex_remap_and_properties() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_vertex_property("label", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("label", a->get_handle(), 10) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("label", b->get_handle(), 20) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto edit = mesh.collapse_edge(edge_ab, a);
    HM_CHECK(edit.ok);
    HM_CHECK(!edit.vertex_handle_remap.empty());
    HM_CHECK(edit.removed_vertices.size() == 1);
    HM_CHECK(edit.removed_vertices.front() == b->get_handle());

    mesh.complete_mesh();
    const auto remapped_a = edit.vertex_handle_remap.at(a->get_handle());
    int label = -1;
    HM_CHECK(mesh.try_get_vertex_property("label", remapped_a, label));
    HM_CHECK(label == 10);
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
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    HM_CHECK(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("marker", x->get_handle(), 55) == halfMesh::PropertyStatus::Added);
    const auto old_x_handle = x->get_handle();
    const auto old_extra_face_handle = mesh.get_faces().back()->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto edit = mesh.collapse_edge(edge_ab, a);
    HM_CHECK(edit.ok);
    mesh.complete_mesh();

    HM_CHECK(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    HM_CHECK(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    HM_CHECK(marker == 55);
    HM_CHECK(mesh.get_face(old_extra_face_handle) != nullptr);
    HM_CHECK(edit.vertex_handle_remap.count(old_x_handle) == 1);
    HM_CHECK(edit.vertex_handle_remap.at(old_x_handle) == old_x_handle);
}

void test_collapse_edit_result_contract() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    HM_CHECK(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    const auto unaffected_vertex = x->get_handle();
    const auto unaffected_face = mesh.get_faces().back()->get_handle();
    const auto unaffected_edge = find_edge_between(mesh, x, y);
    HM_CHECK(unaffected_edge != nullptr);
    const auto unaffected_edge_handle = unaffected_edge->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);

    const auto edit = mesh.collapse_edge(edge_ab, a);
    HM_CHECK(edit.ok);
    mesh.complete_mesh();

    HM_CHECK(are_disjoint(edit.created_vertices, edit.removed_vertices));
    HM_CHECK(are_disjoint(edit.created_edges, edit.removed_edges));
    HM_CHECK(are_disjoint(edit.created_faces, edit.removed_faces));

    HM_CHECK(edit.vertex_handle_remap.count(unaffected_vertex) == 1);
    HM_CHECK(edit.vertex_handle_remap.at(unaffected_vertex) == unaffected_vertex);
    HM_CHECK(edit.edge_handle_remap.count(unaffected_edge_handle) == 1);
    HM_CHECK(edit.edge_handle_remap.at(unaffected_edge_handle) == unaffected_edge_handle);
    HM_CHECK(edit.face_handle_remap.count(unaffected_face) == 1);
    HM_CHECK(edit.face_handle_remap.at(unaffected_face) == unaffected_face);
}
} // namespace

int main() {
    test_collapse_interior_edge_to_endpoint();
    test_collapse_rejects_non_incident_target_vertex();
    test_collapse_rejects_null_target();
    test_collapse_rejects_duplicate_face_result();
    test_collapse_accepts_valid_boundary_link_condition();
    test_collapse_rejects_interior_link_condition_violation();
    test_collapse_rejects_boundary_link_condition_violation();
    test_collapse_edit_result_has_vertex_remap_and_properties();
    test_collapse_keeps_unrelated_handles_stable();
    test_collapse_edit_result_contract();
    std::cout << "halfMesh collapse edit tests passed\n";
    return 0;
}
