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
void test_reject_invalid_face_insertions() {
    halfMesh::triMesh mesh;
    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);
    const auto v4 = mesh.add_vertex(1.0, 1.0, 0.0);

    const auto f0 = mesh.add_face(v1, v2, v3);
    HM_CHECK(f0 != nullptr);

    HM_CHECK(mesh.add_face(v1, v1, v2) == nullptr);
    HM_CHECK(mesh.add_face(v1, v2, v4) == nullptr);

    mesh.complete_mesh();
    HM_CHECK(mesh.is_valid());
    HM_CHECK(mesh.get_faces().size() == 1);
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
    HM_CHECK(mesh.num_connected_components() == 2);

    HM_CHECK(mesh.delete_face(mesh.get_faces().at(1)));
    HM_CHECK(mesh.remove_unreferenced_vertices() == 3);
    const auto a = mesh.add_vertex(10.0, 0.0, 0.0);
    const auto b = mesh.add_vertex(11.0, 0.0, 0.0);
    const auto c = mesh.add_vertex(10.0, 1.0, 0.0);
    HM_CHECK(mesh.add_face(a, b, c) != nullptr);
    mesh.complete_mesh();

    HM_CHECK(mesh.num_connected_components() == 2);
}
} // namespace

int main() {
    test_reject_invalid_face_insertions();
    test_connected_components_after_handle_gaps();
    std::cout << "halfMesh mutation/regression tests passed\n";
    return 0;
}
