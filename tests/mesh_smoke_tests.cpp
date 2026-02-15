#include "halfMesh.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

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

    assert(mesh.get_vertices().size() == 5);
    assert(mesh.get_faces().size() == 3);
    assert(mesh.get_edges().size() == 7);
    assert(mesh.get_half_edges().size() == 9);

    assert(!mesh.is_multiply_connected());
    assert(mesh.compute_number_of_holes() == 1);
    assert(mesh.is_manifold());

    const auto v0 = mesh.get_vertices().at(0);
    const auto v1 = mesh.get_vertices().at(1);
    assert(mesh.one_ring_vertex_of_a_vertex(v0).size() == 1);
    assert(mesh.one_ring_vertex_of_a_vertex(v1).size() == 3);

    const auto f0 = mesh.get_faces().at(0);
    assert(mesh.adjacent_faces(f0).size() == 1);
    assert(mesh.one_ring_faces_of_a_vertex(v1).size() == 3);

    const auto area = mesh.surface_area();
    assert(std::abs(area - 1.0) < 1e-12);

    const auto bbox = mesh.axis_aligned_bounding_box();
    assert((bbox.min() - Eigen::Vector3d(0.0, 0.0, 0.0)).norm() < 1e-12);
    assert((bbox.max() - Eigen::Vector3d(2.5, 0.5, 0.0)).norm() < 1e-12);
}

void test_validation_and_dirty_state() {
    halfMesh::triMesh mesh;
    const auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    const auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    const auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);
    mesh.add_face(v1, v2, v3);

    assert(mesh.is_topology_dirty());
    const auto pre = mesh.validate();
    assert(!pre.ok);

    mesh.complete_mesh();
    assert(!mesh.is_topology_dirty());

    const auto post = mesh.validate();
    assert(post.ok);
    assert(mesh.is_valid());
}

void test_property_guards() {
    auto mesh = build_reference_mesh();

    assert(!mesh.has_vertex_property("temperature"));
    assert(mesh.add_vertex_property("temperature", 0.0) == halfMesh::PropertyStatus::Added);
    assert(mesh.has_vertex_property("temperature"));

    const auto v0 = mesh.get_vertices().at(0)->get_handle();
    assert(mesh.try_set_vertex_property("temperature", v0, 3.14) == halfMesh::PropertyStatus::Added);

    double out = 0.0;
    assert(mesh.try_get_vertex_property("temperature", v0, out));
    assert(std::abs(out - 3.14) < 1e-12);

    assert(mesh.try_set_vertex_property("does_not_exist", v0, 1.0) == halfMesh::PropertyStatus::DoesNotExist);
    assert(!mesh.try_get_vertex_property("does_not_exist", v0, out));

    const unsigned invalid_handle = 999999;
    assert(mesh.try_set_vertex_property("temperature", invalid_handle, 1.0) == halfMesh::PropertyStatus::DoesNotExist);
    assert(!mesh.try_get_vertex_property("temperature", invalid_handle, out));
}
}

int main() {
    test_basic_topology_and_geometry();
    test_validation_and_dirty_state();
    test_property_guards();

    std::cout << "halfMesh smoke tests passed\n";
    return 0;
}
