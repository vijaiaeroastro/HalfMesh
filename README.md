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

Current test binaries (run independently or via `ctest`):
- `halfMesh_test_edit_split`
- `halfMesh_test_edit_collapse`
- `halfMesh_test_edit_flip`
- `halfMesh_test_topology`
- `halfMesh_test_validation`
- `halfMesh_test_mutation`

Core edit implementation files:
- `source/mesh_edit_split.cpp`
- `source/mesh_edit_collapse.cpp`
- `source/mesh_edit_flip.cpp`
- shared helpers: `source/mesh_edit_common.hpp`

Edge split API:
- `split_edge(edge, t)` splits at fraction `t` along the edge (`0 < t < 1`, default `t=0.5`)
- returns `EditResult` with created/removed handles and remap metadata
- local edit: only incident topology is changed; unrelated handles stay stable

Edge collapse API:
- `can_collapse(edge, target_vertex)` checks collapse preconditions
- `collapse_edge(edge, target_vertex)` collapses to one endpoint if valid and returns `EditResult`
- local edit: only source-vertex incident topology is rewritten; unrelated handles stay stable
- enforces link condition: shared endpoint neighbors must match opposite vertices of the collapsed edge's incident face(s)

Edge flip API:
- `can_flip(edge)` checks interior/validity constraints
- `flip_edge(edge)` swaps the diagonal between two incident triangles and returns `EditResult`
- local edit: only the two incident faces plus the flipped edge are changed

`EditResult` includes:
- `ok` and `error`
- `created_*` / `removed_*` handle lists
- `vertex_handle_remap`, `edge_handle_remap`, `face_handle_remap`

## TDD Checklist

Use this loop for every feature, bug fix, or refactor that changes behavior:

1. `Red`: add or update a focused test first in `tests/test_<feature>.cpp`.
2. Run only that test target and confirm it fails for the expected reason.
3. `Green`: implement the minimal production change to make the test pass.
4. Run the full suite with `ctest --test-dir build --output-on-failure`.
5. `Refactor`: clean up code while keeping tests green.
6. Add/extend an example in `examples/` when behavior is user-facing.
7. Update docs (`README.md`) if API, constraints, or workflow changed.

Conventions:
- Prefer many small tests over one large scenario test.
- Keep test names behavior-focused (`test_reject_invalid_face_insertions`).
- Every bug fix should include a regression test.

### Build and run the example

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build
./build/halfMesh_example
./build/halfMesh_validation_example
./build/halfMesh_edge_split_example
./build/halfMesh_edge_collapse_example
./build/halfMesh_edge_flip_example
```

### Build and run benchmark

```bash
cmake -S . -B build -DBUILD_BENCHMARKS=ON
cmake --build build
./build/halfMesh_bench_local_edits
```

Benchmark output reports attempted/succeeded operation counts, wall-clock time, and ops/sec for:
- `split_edge`
- `collapse_edge`
- `flip_edge`

Benchmark CLI options:
- `--profile quick` (default)
- `--profile stress`
- `--nx <int> --ny <int>`
- `--split-iters <int> --collapse-iters <int> --flip-iters <int>`
- `--json` (machine-readable output)

Examples:
```bash
# Quick profile (default)
./build/halfMesh_bench_local_edits --profile quick

# Stress profile
./build/halfMesh_bench_local_edits --profile stress

# Custom run
./build/halfMesh_bench_local_edits --nx 48 --ny 48 --split-iters 800 --collapse-iters 400 --flip-iters 2500

# JSON output
./build/halfMesh_bench_local_edits --json
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

## License

MIT
