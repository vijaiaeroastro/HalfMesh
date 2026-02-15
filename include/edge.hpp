// edge.hpp
#pragma once
#include <common.hpp>
#include <limits>

namespace halfMesh {
    class edge {
    public:
        edge(const std::shared_ptr<vertex>& a,
             const std::shared_ptr<vertex>& b)
            : v1_(a),
              v2_(b) {
        }

        ~edge() = default;

        //— Accessors ——
        std::shared_ptr<vertex> get_vertex_one() const { return v1_.lock(); }
        std::shared_ptr<vertex> get_vertex_two() const { return v2_.lock(); }
        unsigned int get_handle() const { return handle_; }
        bool is_boundary() const { return boundary_; }
        std::shared_ptr<halfedge> get_one_half_edge() const { return one_half_edge_.lock(); }

        //— Mutators ——
        void set_handle(const unsigned int h) { handle_ = h; }
        void set_boundary(const bool b) { boundary_ = b; }
        void set_one_half_edge(const std::shared_ptr<halfedge>& he) { one_half_edge_ = he; }

    private:
        std::weak_ptr<vertex> v1_, v2_;
        unsigned int handle_ = std::numeric_limits<unsigned>::max();
        std::weak_ptr<halfedge> one_half_edge_;
        bool boundary_ = false;
    };
} // namespace HalfMesh
