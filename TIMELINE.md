# halfMesh Timeline: Now → Finish Line

This file defines a single end-to-end execution timeline to take **halfMesh** from current foundation status to a robust library suitable for both:

1. **Combinatorial/topological research workflows**, and
2. **DDG (Discrete Differential Geometry) and geometry processing workflows**.

---

## Current Status (Baseline)

Completed recently:
- Library-first CMake + CI + example/test harness
- Initial validation/report system
- Dirty topology tracking
- Safer property/handle guard APIs

This timeline starts from that baseline.

---

## Program Structure

- **Total horizon:** ~20 weeks
- **Cadence:** 1 week sprints, with phase gates
- **Definition of done per phase:**
  - API documented
  - tests + CI green
  - examples added
  - benchmark/checks where relevant

---

## Phase 1 — Kernel Hardening Completion (Weeks 1–4)

### Goal
Make connectivity kernel deterministic, safe, and stable for aggressive edits.

### Workstreams
1. **Traversal canonicalization**
   - Use half-edge `next()/prev()` as canonical traversal source.
   - Remove traversal-by-vertex-search where it can desync.

2. **Finalization contract**
   - Define behavior for topology queries on dirty meshes:
     - strict mode: reject queries,
     - compatibility mode: optional auto-finalize.

3. **Validation expansion**
   - Extend validation report checks for:
     - edge/face ownership consistency,
     - boundary loop consistency,
     - orphaned handle-map entries,
     - null parent/representative constraints.

4. **Test deepening**
   - Add negative/broken-state tests and delete-heavy scenarios.
   - Add disconnected and boundary-dominant fixture meshes.

### Exit Criteria
- `validate()` catches seeded corruptions.
- All topology helpers have defined dirty-state behavior.
- CI green with expanded tests.

---

## Phase 2 — Combinatorial Editing Core (Weeks 5–8)

### Goal
Implement robust local mesh editing primitives with strict precondition checks.

### Workstreams
1. **Operation contracts (`can_*`)**
   - `can_flip(edge)`
   - `can_split(edge)`
   - `can_collapse(edge, target)`

2. **Primitive implementations**
   - edge flip
   - edge split
   - edge collapse (with link condition)

3. **Property propagation hooks**
   - default interpolation behavior for split/collapse.
   - callback mechanism for custom attribute propagation.

4. **Transactional editing**
   - begin/apply/rollback style API for batches.

### Exit Criteria
- Operators succeed/fail deterministically.
- No invalid states after successful operations (`validate().ok == true`).
- Operator-specific regression tests included.

---

## Phase 3 — Topology Utilities & Research Ergonomics (Weeks 9–11)

### Goal
Make halfMesh productive for combinatorial experiments and algorithm prototyping.

### Workstreams
1. **Circulators/iterators**
   - vertex→vertex, vertex→face, face→halfedge, boundary-loop iterator.

2. **Submesh/component tools**
   - extract connected component
   - boundary extraction
   - star/link utilities

3. **Canonical reindexing / compactification**
   - optional garbage collection with remap tables.

4. **I/O robustness + metadata**
   - preserve/roundtrip basic metadata where possible.

### Exit Criteria
- Research-style scripts can implement common combinatorial algorithms without internal hacks.
- Stable/clear behavior for handle remapping and compactification.

---

## Phase 4 — DDG Operator Layer (Weeks 12–15)

### Goal
Add foundational linear operators for geometry processing.

### Workstreams
1. **Geometric primitives**
   - robust face/vertex area variants,
   - cotangent weights,
   - angle defects,
   - boundary-aware quantities.

2. **Core sparse operators**
   - mass matrix (lumped + mixed variants)
   - cotan Laplace-Beltrami
   - gradient/divergence assembly

3. **Boundary conditions**
   - Dirichlet/Neumann masks and helper APIs.

4. **Validation & numerical checks**
   - compare against known meshes/problems.

### Exit Criteria
- Operator outputs verified on canonical examples.
- Sparse assembly APIs stable and documented.

---

## Phase 5 — DDG Algorithms (Weeks 16–18)

### Goal
Ship practical higher-level DDG algorithms on top of Phase 4 operators.

### Target Algorithms
1. Laplacian smoothing (explicit/implicit)
2. Poisson solve on mesh signals
3. Mean curvature flow
4. Heat-method geodesics

### Exit Criteria
- Each algorithm has:
  - tested API,
  - example program,
  - basic result validation.

---

## Phase 6 — Performance, Packaging, and Release (Weeks 19–20)

### Goal
Prepare production-quality release.

### Workstreams
1. **Performance pass**
   - profile hotspots,
   - remove avoidable allocations,
   - benchmark kernels/operators.

2. **Docs finalization**
   - user guide,
   - API reference,
   - migration notes.

3. **Release engineering**
   - semantic versioning policy,
   - changelog,
   - release checklist,
   - tagged v1.0.0 release.

### Exit Criteria
- Performance baseline documented.
- Docs complete for DS + DDG tracks.
- v1.0.0 ready to publish.

---

## Parallel Tracks (Run Across All Phases)

### A) Quality & CI
- Expand CI matrix (compilers/build types/sanitizers).
- Add static analysis and formatting checks.
- Keep tests deterministic and fast.

### B) API Stability
- Deprecation policy for API changes.
- Keep sharp edges behind explicit opt-in APIs.

### C) Examples & Adoption
- Maintain minimal + advanced examples each phase.
- Track usage pain points and feed backlog.

---

## Milestone Map (Quick View)

- **M1 (Week 4):** Hardened kernel + reliable validation
- **M2 (Week 8):** Flip/split/collapse editing core
- **M3 (Week 11):** Research-friendly topology utilities
- **M4 (Week 15):** Core DDG operator layer
- **M5 (Week 18):** Practical DDG algorithms
- **M6 (Week 20):** Performance pass + v1.0.0 release readiness

---

## Finish Line Definition

halfMesh is considered at finish-line quality when:

1. Core topology/editing operations are robust and validated.
2. DDG core operators and flagship algorithms are implemented and tested.
3. CI and benchmarks provide confidence in correctness/performance.
4. Documentation supports both research and production users.
5. v1.0.0 release is published with migration/versioning policy.

