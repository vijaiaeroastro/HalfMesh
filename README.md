# halfMesh

A lightweight half-edge mesh library for triangular surface meshes.

## What is implemented (essentials)

- Half-edge connectivity with vertex/edge/face handles
- Traversal helpers (adjacent faces, one-ring neighborhoods)
- Topology checks (manifold/oriented/boundary/component queries)
- Geometric queries (surface area, normals, axis-aligned bounding box)
- Property storage on vertices/edges/faces via JSON values
- I/O support for STL (ASCII/Binary), GMSH v2, VTK export, OBJ export, and BSON-based `.bm`

## Build

```bash
cmake -S . -B build
cmake --build build
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Build and run the example

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build
./build/halfMesh_example
```

## Minimal usage

```cpp
#include "halfMesh.hpp"

int main() {
    halfMesh::triMesh mesh;

    auto v1 = mesh.add_vertex(0.0, 0.0, 0.0);
    auto v2 = mesh.add_vertex(1.0, 0.0, 0.0);
    auto v3 = mesh.add_vertex(0.0, 1.0, 0.0);

    mesh.add_face(v1, v2, v3);
    mesh.complete_mesh();
    mesh.save("mesh.stl");
}
```

## Install library and headers

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/tmp/halfmesh-install
cmake --build build
cmake --install build
```

## Dependencies

- C++17+
- `nlohmann/json` (vendored in `deps/nlohmann-3.12.0`)
- Eigen 3.4.0 (vendored in `deps/eigen-3.4.0`)

## License

MIT
