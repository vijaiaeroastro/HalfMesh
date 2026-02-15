#include "triMesh.hpp"
#include <vector>
#include <unordered_set>

namespace halfMesh {
    halfEdgePtr triMesh::get_next_half_edge(const halfEdgePtr &he,
                                                     const facePtr &f) const {
        if (!he || !f) {
            return nullptr;
        }

        if (const auto candidate = he->next();
            candidate && candidate->get_parent_face() == f) {
            return candidate;
        }

        const auto v2 = he->get_vertex_two();
        if (!v2) {
            return nullptr;
        }
        for (auto &candidate: v2->get_outgoing_half_edges()) {
            if (candidate != he && candidate->get_parent_face() == f)
                return candidate;
        }
        return nullptr;
    }

    halfEdgePtr triMesh::get_previous_half_edge(const halfEdgePtr &he,
                                                         const facePtr &f) const {
        if (!he || !f) {
            return nullptr;
        }

        if (const auto candidate = he->prev();
            candidate && candidate->get_parent_face() == f) {
            return candidate;
        }

        const auto v1 = he->get_vertex_one();
        if (!v1) {
            return nullptr;
        }
        for (auto &candidate: v1->get_incoming_half_edges()) {
            if (candidate != he && candidate->get_parent_face() == f)
                return candidate;
        }
        return nullptr;
    }

    facePtr triMesh::get_one_neighbour_face(const facePtr &f) const {
        const auto opposingHalfEdge = f->get_one_half_edge()->get_opposing_half_edge();
        if (! opposingHalfEdge) {
            return f;
        }
        return opposingHalfEdge->get_parent_face();
    }

    std::unordered_set<vertexPtr> triMesh::one_ring_vertex_of_a_vertex(const vertexPtr &v) const {
        std::unordered_set<vertexPtr> out;
        if (!v) {
            return out;
        }
        for (const auto &he: v->get_outgoing_half_edges()) {
            out.insert(he->get_vertex_two());
        }
        for (const auto &he: v->get_incoming_half_edges()) {
            out.insert(he->get_vertex_one());
        }
        out.erase(nullptr);
        return out;
    }

    std::unordered_set<facePtr> triMesh::one_ring_faces_of_a_vertex(const vertexPtr &v) const {
        std::unordered_set<facePtr> out;
        for (const auto &he: v->get_outgoing_half_edges()) {
            out.insert(he->get_parent_face());
        }
        for (const auto &he: v->get_incoming_half_edges()) {
            out.insert(he->get_parent_face());
        }
        return out;
    }

    std::unordered_set<facePtr> triMesh::adjacent_faces(const facePtr &f) const {
        std::unordered_set<facePtr> out;
        if (!f) {
            return out;
        }
        auto he = f->get_one_half_edge();
        for (int i = 0; i < 3 && he; ++i) {
            // cross to the opposite
            const auto opposingHalfEdge = he->get_opposing_half_edge();
            if (opposingHalfEdge) {
                // skip if it's the same face
                if (auto nf = opposingHalfEdge->get_parent_face(); nf && nf->get_handle() != f->get_handle()) {
                out.insert(nf);
                }
            }
            he = he->next();
        }
        return out;
    }
} // namespace HalfMesh
