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

void test_split_boundary_edge() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(mesh.can_split(edge_ab));

    const auto edit = mesh.split_edge(edge_ab);
    HM_CHECK(edit.ok);
    HM_CHECK(edit.created_vertices.size() == 1);
    const auto m = mesh.get_vertex(edit.created_vertices.front());
    HM_CHECK(m != nullptr);
    mesh.complete_mesh();

    HM_CHECK(std::abs(m->get_x() - 1.0) < 1e-12);
    HM_CHECK(std::abs(m->get_y() - 0.0) < 1e-12);
    HM_CHECK(std::abs(m->get_z() - 0.0) < 1e-12);

    HM_CHECK(mesh.get_faces().size() == 2);
    HM_CHECK(mesh.get_edges().size() == 5);
    HM_CHECK(mesh.get_vertices().size() == 4);
    HM_CHECK(mesh.validate().ok);
}

void test_split_interior_edge() {
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
    HM_CHECK(mesh.can_split(edge_ab));

    const auto edit = mesh.split_edge(edge_ab);
    HM_CHECK(edit.ok);
    HM_CHECK(edit.created_vertices.size() == 1);
    HM_CHECK(mesh.get_vertex(edit.created_vertices.front()) != nullptr);

    mesh.complete_mesh();
    HM_CHECK(mesh.get_faces().size() == 4);
    HM_CHECK(mesh.get_edges().size() == 8);
    HM_CHECK(mesh.get_vertices().size() == 5);
    HM_CHECK(mesh.validate().ok);
}

void test_split_edge_fraction_parameter() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);

    const auto edit = mesh.split_edge(edge_ab, 0.25);
    HM_CHECK(edit.ok);
    HM_CHECK(edit.created_vertices.size() == 1);
    const auto p = mesh.get_vertex(edit.created_vertices.front());
    HM_CHECK(p != nullptr);
    mesh.complete_mesh();

    HM_CHECK(std::abs(p->get_x() - 1.0) < 1e-12);
    HM_CHECK(std::abs(p->get_y() - 0.0) < 1e-12);
    HM_CHECK(std::abs(p->get_z() - 0.0) < 1e-12);
    HM_CHECK(mesh.validate().ok);
}

void test_split_edge_fraction_rejects_endpoints() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(2.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);

    HM_CHECK(!mesh.split_edge(edge_ab, 0.0).ok);
    HM_CHECK(!mesh.split_edge(edge_ab, 1.0).ok);
    HM_CHECK(!mesh.split_edge(edge_ab, -0.1).ok);
    HM_CHECK(!mesh.split_edge(edge_ab, 1.1).ok);

    mesh.complete_mesh();
    HM_CHECK(mesh.get_vertices().size() == 3);
    HM_CHECK(mesh.get_faces().size() == 1);
    HM_CHECK(mesh.validate().ok);
}

void test_split_rejects_invalid_edge_handle() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    auto bogus = std::make_shared<halfMesh::edge>(a, b);
    bogus->set_handle(999999u);
    HM_CHECK(!mesh.can_split(bogus));
    HM_CHECK(!mesh.split_edge(bogus, 0.5).ok);
}

void test_split_edit_result_and_property_propagation() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto f = mesh.add_face(a, b, c);
    HM_CHECK(f != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_vertex_property("temperature", 0.0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("temperature", a->get_handle(), 0.0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("temperature", b->get_handle(), 8.0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.add_face_property("region", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_face_property("region", f->get_handle(), 7) == halfMesh::PropertyStatus::Added);

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    HM_CHECK(mesh.add_edge_property("weight", 0.0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_edge_property("weight", edge_ab->get_handle(), 3.5) == halfMesh::PropertyStatus::Added);

    const auto edit = mesh.split_edge(edge_ab, 0.25);
    HM_CHECK(edit.ok);
    HM_CHECK(edit.created_vertices.size() == 1);
    HM_CHECK(!edit.created_edges.empty());
    HM_CHECK(edit.removed_edges.size() == 1);
    HM_CHECK(edit.removed_faces.size() == 1);
    HM_CHECK(edit.created_faces.size() == 2);

    mesh.complete_mesh();
    const auto mid = mesh.get_vertex(edit.created_vertices.front());
    HM_CHECK(mid != nullptr);

    double mid_temperature = -1.0;
    HM_CHECK(mesh.try_get_vertex_property("temperature", mid->get_handle(), mid_temperature));
    HM_CHECK(std::abs(mid_temperature - 2.0) < 1e-12);

    for (const auto new_face_handle: edit.created_faces) {
        int region = -1;
        HM_CHECK(mesh.try_get_face_property("region", new_face_handle, region));
        HM_CHECK(region == 7);
    }
}

void test_split_keeps_unrelated_handles_stable() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.add_vertex_property("marker", 0) == halfMesh::PropertyStatus::Added);
    HM_CHECK(mesh.try_set_vertex_property("marker", x->get_handle(), 77) == halfMesh::PropertyStatus::Added);

    const auto old_x_handle = x->get_handle();
    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto edit = mesh.split_edge(edge_ab, 0.5);
    HM_CHECK(edit.ok);
    mesh.complete_mesh();

    HM_CHECK(mesh.get_vertex(old_x_handle) != nullptr);
    int marker = -1;
    HM_CHECK(mesh.try_get_vertex_property("marker", old_x_handle, marker));
    HM_CHECK(marker == 77);
    HM_CHECK(edit.vertex_handle_remap.count(old_x_handle) == 1);
    HM_CHECK(edit.vertex_handle_remap.at(old_x_handle) == old_x_handle);
}

void test_split_edit_result_contract() {
    halfMesh::triMesh mesh;
    const auto a = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(4.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(0.0, 2.0, 0.0);
    const auto x = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto y = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto z = mesh.add_vertex(10.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    HM_CHECK(mesh.add_face(x, y, z) != nullptr);
    mesh.complete_mesh();

    const auto unaffected_vertex = x->get_handle();
    const auto unaffected_face = mesh.get_faces().back()->get_handle();
    const auto unaffected_edge = find_edge_between(mesh, x, y);
    HM_CHECK(unaffected_edge != nullptr);
    const auto unaffected_edge_handle = unaffected_edge->get_handle();

    const auto edge_ab = find_edge_between(mesh, a, b);
    HM_CHECK(edge_ab != nullptr);
    const auto source_edge_handle = edge_ab->get_handle();

    const auto edit = mesh.split_edge(edge_ab, 0.5);
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

    HM_CHECK(edit.edge_handle_remap.count(source_edge_handle) == 0);
}
} // namespace

int main() {
    test_split_boundary_edge();
    test_split_interior_edge();
    test_split_edge_fraction_parameter();
    test_split_edge_fraction_rejects_endpoints();
    test_split_rejects_invalid_edge_handle();
    test_split_edit_result_and_property_propagation();
    test_split_keeps_unrelated_handles_stable();
    test_split_edit_result_contract();
    std::cout << "halfMesh split edit tests passed\n";
    return 0;
}
