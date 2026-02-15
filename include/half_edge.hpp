// half_edge.hpp
#pragma once
#include <common.hpp>
#include <limits>

namespace halfMesh {
    class halfedge {
    public:
        halfedge(vertexPtr a, vertexPtr b)
            : v1_(a), v2_(b) {
        }

        ~halfedge() = default;

        //— Handle for external indexing / properties ——
        unsigned int get_handle() const { return handle_; }
        void set_handle(const unsigned int h) { handle_ = h; }

        //— Vertex ends ——
        vertexPtr get_vertex_one() const { return v1_.lock(); }
        vertexPtr get_vertex_two() const { return v2_.lock(); }

        //— Connectivity pointers ——
        edgePtr get_parent_edge() const { return parent_edge_.lock(); }
        facePtr get_parent_face() const { return parent_face_.lock(); }
        halfEdgePtr get_opposing_half_edge() const { return opposing_.lock(); }

        void set_parent_edge(const edgePtr &e) { parent_edge_ = e; }
        void set_parent_face(const facePtr &f) { parent_face_ = f; }
        void set_opposing_half_edge(const halfEdgePtr &he) { opposing_ = he; }

        //— Boundary flag (you can compute it once after linking) ——
        bool is_boundary() const { return boundary_; }
        void set_boundary(const bool b) { boundary_ = b; }

        // ——— new next/prev API ———
        halfEdgePtr next() const   { return next_.lock(); }
        halfEdgePtr prev() const   { return prev_.lock(); }

        void set_next(const halfEdgePtr& h) { next_ = h; }
        void set_prev(const halfEdgePtr& h) { prev_ = h; }

    private:
        // ownership of end‐points lives in triMesh::vertices_
        std::weak_ptr<vertex> v1_, v2_;

        // integer handle for external indexing/properties
        unsigned int handle_ = std::numeric_limits<unsigned>::max();

        // pointer‐based connectivity
        std::weak_ptr<edge> parent_edge_;
        std::weak_ptr<face> parent_face_;
        std::weak_ptr<halfedge> opposing_;

        bool boundary_ = false;

        // <— new fields —
        std::weak_ptr<halfedge> next_, prev_;
    };
} // namespace HalfMesh
