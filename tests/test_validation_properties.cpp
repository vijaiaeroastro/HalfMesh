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

void test_format_detection_api() {
    using halfMesh::MeshFormat;

    assert(halfMesh::detect_format_from_path("mesh.stl") == MeshFormat::Stl);
    assert(halfMesh::detect_format_from_path("mesh.STL") == MeshFormat::Stl);
    assert(halfMesh::detect_format_from_path("mesh.obj") == MeshFormat::Obj);
    assert(halfMesh::detect_format_from_path("mesh.msh") == MeshFormat::Gmsh);
    assert(halfMesh::detect_format_from_path("mesh.bm") == MeshFormat::Binary);
    assert(halfMesh::detect_format_from_path("mesh.vtk") == MeshFormat::Vtk);
    assert(halfMesh::detect_format_from_path("mesh.unknown") == MeshFormat::Unknown);

    // Backward-compatibility shim should still behave the same.
    assert(halfMesh::guess_mesh_format("legacy.stl") == MeshFormat::Stl);

    halfMesh::MeshType legacyType = halfMesh::guess_mesh_format("legacy.obj");
    assert(legacyType == MeshFormat::Obj);
}
} // namespace

int main() {
    test_validation_and_dirty_state();
    test_property_guards();
    test_format_detection_api();
    std::cout << "halfMesh validation/property tests passed\n";
    return 0;
}
