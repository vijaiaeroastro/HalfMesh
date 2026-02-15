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

void test_flip_interior_edge() {
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
    HM_CHECK(mesh.can_flip(edge_ab));
    HM_CHECK(mesh.flip_edge(edge_ab).ok);

    mesh.complete_mesh();
    HM_CHECK(mesh.get_vertices().size() == 4);
    HM_CHECK(mesh.get_faces().size() == 2);
    HM_CHECK(find_edge_between(mesh, c, d) != nullptr);
    HM_CHECK(find_edge_between(mesh, a, b) == nullptr);
    HM_CHECK(mesh.validate().ok);
}

void test_flip_rejects_boundary_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(!mesh.can_flip(edge_ab));
    HM_CHECK(!mesh.flip_edge(edge_ab).ok);
}

void test_flip_rejects_duplicate_face_result() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    HM_CHECK(mesh.add_face(c, d, a) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);

    HM_CHECK(!mesh.can_flip(edge_ab));
    HM_CHECK(!mesh.flip_edge(edge_ab).ok);
}

void test_flip_rejects_existing_diagonal_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(5.0, 0.0, 0.0);

    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    HM_CHECK(mesh.add_face(c, d, x) != nullptr); // creates edge c-d already
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(!mesh.can_flip(edge_ab));
    HM_CHECK(!mesh.flip_edge(edge_ab).ok);
}

void test_flip_rejects_invalid_edge_handle() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    auto bogus = std::make_shared<halfMesh::edge>(a, b);
    bogus->set_handle(999999u);
    HM_CHECK(!mesh.can_flip(bogus));
    HM_CHECK(!mesh.flip_edge(bogus).ok);
}

void test_flip_edit_result_and_face_property_remap() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto d = mesh.add_vertex(2.0, 2.0, 0.0);
    const auto f0 = mesh.add_face(a, b, c);
    const auto f1 = mesh.add_face(b, a, d);
    HM_CHECK(f0 != nullptr);
    HM_CHECK(f1 != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_face_property("id", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_face_property("id", f0->get_handle(), 101) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_face_property("id", f1->get_handle(), 202) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto edit = mesh.flip_edge(edge_ab);
    HM_CHECK(edit.ok);
    HM_CHECK(edit.removed_faces.size() == 2);
    HM_CHECK(edit.created_faces.size() == 2);

    mesh.complete_mesh();
    HM_CHECK(find_edge_between(mesh, c, d) != nullptr);

    const auto remapped_f0 = edit.face_handle_remap.at(f0->get_handle());
    const auto remapped_f1 = edit.face_handle_remap.at(f1->get_handle());
    int id0 = -1;
    int id1 = -1;
    HM_CHECK(mesh.try_get_face_property("id", remapped_f0, id0));
    HM_CHECK(mesh.try_get_face_property("id", remapped_f1, id1));
    HM_CHECK(id0 == 101);
    HM_CHECK(id1 == 202);
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
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(b, a, d) != nullptr);
    HM_CHECK(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("marker", x->get_handle(), 99) == halfMesh::PropertyStatus::Added);

    const auto old_x_handle = x->get_handle();
    const auto old_extra_face_handle = mesh.get_faces().back()->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto edit = mesh.flip_edge(edge_ab);
    HM_CHECK(edit.ok);
    mesh.complete_mesh();

    HM_CHECK(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    HM_CHECK(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    HM_CHECK(marker == 99);
    HM_CHECK(mesh.get_face(old_extra_face_handle) != nullptr);
}

void test_flip_edit_result_contract() {
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
    const auto old_flipped_edge_handle = edge_ab->get_handle();

    const auto edit = mesh.flip_edge(edge_ab);
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

    HM_CHECK(edit.edge_handle_remap.count(old_flipped_edge_handle) == 1);
    HM_CHECK(edit.edge_handle_remap.at(old_flipped_edge_handle) != old_flipped_edge_handle);
}
} // namespace

int main() {
    test_flip_interior_edge();
    test_flip_rejects_boundary_edge();
    test_flip_rejects_duplicate_face_result();
    test_flip_rejects_existing_diagonal_edge();
    test_flip_rejects_invalid_edge_handle();
    test_flip_edit_result_and_face_property_remap();
    test_flip_keeps_unrelated_handles_stable();
    test_flip_edit_result_contract();
    std::cout << "halfMesh flip edit tests passed\n";
    return 0;
}
