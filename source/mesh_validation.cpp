#include "triMesh.hpp"

namespace halfMesh {
    namespace {
        void add_issue(MeshValidationReport &report, std::string code, std::string detail) {
            report.ok = false;
            report.issues.push_back({std::move(code), std::move(detail)});
        }
    }

    MeshValidationReport triMesh::validate() const {
        MeshValidationReport report{};

        if (topology_dirty_) {
            add_issue(report, "TOPOLOGY_DIRTY", "Mesh topology is dirty; call complete_mesh() before topology queries.");
        }

        if (vertices_.size() != handle_to_vertex_.size()) {
            add_issue(report, "VERTEX_MAP_SIZE_MISMATCH", "vertices_ and handle_to_vertex_ have different sizes.");
        }
        if (edges_.size() != handle_to_edge_.size()) {
            add_issue(report, "EDGE_MAP_SIZE_MISMATCH", "edges_ and handle_to_edge_ have different sizes.");
        }
        if (faces_.size() != handle_to_face_.size()) {
            add_issue(report, "FACE_MAP_SIZE_MISMATCH", "faces_ and handle_to_face_ have different sizes.");
        }
        if (half_edges_.size() != handle_to_half_edge_.size()) {
            add_issue(report, "HALFEDGE_MAP_SIZE_MISMATCH", "half_edges_ and handle_to_half_edge_ have different sizes.");
        }

        for (const auto &v: vertices_) {
            if (!v) {
                add_issue(report, "NULL_VERTEX", "vertices_ contains nullptr.");
                continue;
            }
            if (!has_vertex_handle(v->get_handle())) {
                add_issue(report, "MISSING_VERTEX_HANDLE", "vertex handle missing from map.");
                continue;
            }
            if (get_vertex(v->get_handle()) != v) {
                add_issue(report, "VERTEX_MAP_POINTER_MISMATCH", "vertex handle maps to a different object.");
            }
            for (const auto &he: v->get_outgoing_half_edges()) {
                if (!he || !he->get_vertex_one() || he->get_vertex_one()->get_handle() != v->get_handle()) {
                    add_issue(report, "VERTEX_OUTGOING_MISMATCH", "outgoing half-edge does not start at the owning vertex.");
                }
            }
            for (const auto &he: v->get_incoming_half_edges()) {
                if (!he || !he->get_vertex_two() || he->get_vertex_two()->get_handle() != v->get_handle()) {
                    add_issue(report, "VERTEX_INCOMING_MISMATCH", "incoming half-edge does not end at the owning vertex.");
                }
            }
        }

        for (const auto &he: half_edges_) {
            if (!he) {
                add_issue(report, "NULL_HALFEDGE", "half_edges_ contains nullptr.");
                continue;
            }
            if (!has_half_edge_handle(he->get_handle())) {
                add_issue(report, "MISSING_HALFEDGE_HANDLE", "half-edge handle missing from map.");
                continue;
            }
            if (get_half_edge(he->get_handle()) != he) {
                add_issue(report, "HALFEDGE_MAP_POINTER_MISMATCH", "half-edge handle maps to a different object.");
            }

            const auto v1 = he->get_vertex_one();
            const auto v2 = he->get_vertex_two();
            if (!v1 || !v2) {
                add_issue(report, "HALFEDGE_NULL_ENDPOINT", "half-edge has null endpoint vertex.");
            }

            if (const auto opp = he->get_opposing_half_edge()) {
                if (opp->get_opposing_half_edge() != he) {
                    add_issue(report, "HALFEDGE_OPPOSITE_NOT_SYMMETRIC", "he->opp->opp != he.");
                }
            }

            if (const auto next = he->next()) {
                if (next->prev() != he) {
                    add_issue(report, "HALFEDGE_NEXT_PREV_BROKEN", "next->prev != he.");
                }
            }
            if (const auto prev = he->prev()) {
                if (prev->next() != he) {
                    add_issue(report, "HALFEDGE_PREV_NEXT_BROKEN", "prev->next != he.");
                }
            }

            if (he->is_boundary() && he->get_opposing_half_edge()) {
                add_issue(report, "BOUNDARY_WITH_OPPOSITE", "boundary half-edge has an opposite half-edge.");
            }
        }

        for (const auto &e: edges_) {
            if (!e) {
                add_issue(report, "NULL_EDGE", "edges_ contains nullptr.");
                continue;
            }
            if (!has_edge_handle(e->get_handle())) {
                add_issue(report, "MISSING_EDGE_HANDLE", "edge handle missing from map.");
                continue;
            }
            if (get_edge(e->get_handle()) != e) {
                add_issue(report, "EDGE_MAP_POINTER_MISMATCH", "edge handle maps to a different object.");
            }

            const auto one = e->get_one_half_edge();
            if (!one) {
                add_issue(report, "EDGE_WITHOUT_HALFEDGE", "edge missing representative half-edge.");
            } else {
                if (one->get_parent_edge() != e) {
                    add_issue(report, "EDGE_PARENT_MISMATCH", "edge representative half-edge does not point back to edge.");
                }
                if (e->is_boundary() != one->is_boundary()) {
                    add_issue(report, "EDGE_BOUNDARY_MISMATCH", "edge boundary flag does not match representative half-edge.");
                }
            }
        }

        for (const auto &f: faces_) {
            if (!f) {
                add_issue(report, "NULL_FACE", "faces_ contains nullptr.");
                continue;
            }
            if (!has_face_handle(f->get_handle())) {
                add_issue(report, "MISSING_FACE_HANDLE", "face handle missing from map.");
                continue;
            }
            if (get_face(f->get_handle()) != f) {
                add_issue(report, "FACE_MAP_POINTER_MISMATCH", "face handle maps to a different object.");
            }

            const auto start = f->get_one_half_edge();
            if (!start) {
                add_issue(report, "FACE_WITHOUT_HALFEDGE", "face has no representative half-edge.");
                continue;
            }

            auto he = start;
            bool cycle_ok = true;
            for (int i = 0; i < 3; ++i) {
                if (!he || he->get_parent_face() != f) {
                    cycle_ok = false;
                    break;
                }
                he = he->next();
            }
            if (!cycle_ok || he != start) {
                add_issue(report, "FACE_CYCLE_BROKEN", "face half-edge cycle is not a closed 3-cycle.");
            }
        }

        return report;
    }
} // namespace halfMesh
