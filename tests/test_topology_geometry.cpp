#include "halfMesh.hpp"

#include <cstdlib>
#include <iostream>

inline void hm_check(bool cond, const char *expr, const char *file, int line) {
    if (!cond) {
        std::cerr << "HM_CHECK failed: " << expr << " at " << file << ":" << line << "\n";
        std::abort();
    }
}
#define HM_CHECK(expr) hm_check((expr), #expr, __FILE__, __LINE__)

namespace {
halfMesh::triMesh build_reference_mesh() {
    halfMesh::triMesh mesh;

    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 0.5, 0.0);
    const auto v4 = mesh.add_vertex(1.5, 0.5, 0.0);
    const auto v5 = mesh.add_vertex(2.5, 0.0, 0.0);

    mesh.add_face(v1, v2, v3);
    mesh.add_face(v2, v4, v3);
    mesh.add_face(v2, v5, v4);
    mesh.complete_mesh();

    return mesh;
}

void test_basic_topology_and_geometry() {
    const auto mesh = build_reference_mesh();

    HM_CHECK(mesh.get_vertices().size() == 5);
    HM_CHECK(mesh.get_faces().size() == 3);
    HM_CHECK(mesh.get_edges().size() == 7);
    HM_CHECK(mesh.get_half_edges().size() == 9);

    HM_CHECK(!mesh.is_multiply_connected());
    HM_CHECK(mesh.compute_number_of_holes() == 1);
    HM_CHECK(mesh.is_manifold());

    const auto v0 = mesh.get_vertices().at(0);
    const auto v1 = mesh.get_vertices().at(1);
    HM_CHECK(mesh.one_ring_vertex_of_a_vertex(v0).size() == 2);
    HM_CHECK(mesh.one_ring_vertex_of_a_vertex(v1).size() == 4);

    const auto f0 = mesh.get_faces().at(0);
    HM_CHECK(mesh.adjacent_faces(f0).size() == 1);
    HM_CHECK(mesh.one_ring_faces_of_a_vertex(v1).size() == 3);

    const auto area = mesh.surface_area();
    HM_CHECK(std::abs(area - 1.0) < 1e-12);

    const auto bbox = mesh.axis_aligned_bounding_box();
    HM_CHECK(!bbox.empty);
    HM_CHECK(std::abs(bbox.min_corner[0] - 0.0) < 1e-12);
    HM_CHECK(std::abs(bbox.min_corner[1] - 0.0) < 1e-12);
    HM_CHECK(std::abs(bbox.min_corner[2] - 0.0) < 1e-12);
    HM_CHECK(std::abs(bbox.max_corner[0] - 2.5) < 1e-12);
    HM_CHECK(std::abs(bbox.max_corner[1] - 0.5) < 1e-12);
    HM_CHECK(std::abs(bbox.max_corner[2] - 0.0) < 1e-12);
}
} // namespace

int main() {
    test_basic_topology_and_geometry();
    std::cout << "halfMesh topology/geometry tests passed\n";
    return 0;
}
