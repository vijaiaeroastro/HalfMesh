// face.hpp
#pragma once
#include <common.hpp>
#include <limits>
#include <tuple>

namespace halfMesh {
    class face {
    public:
        face(std::shared_ptr<vertex> a,
             std::shared_ptr<vertex> b,
             std::shared_ptr<vertex> c)
            : v1_(a),
              v2_(b),
              v3_(c) {
        }

        ~face() = default;

        //— Accessors ——
        unsigned int get_handle() const { return handle_; }
        bool is_valid() const { return handle_ != std::numeric_limits<unsigned int>::max(); }

        std::tuple<std::shared_ptr<vertex>,
            std::shared_ptr<vertex>,
            std::shared_ptr<vertex> >
        get_vertices() const {
            return {v1_.lock(), v2_.lock(), v3_.lock()};
        }

        std::shared_ptr<halfedge> get_one_half_edge() const { return one_half_edge_.lock(); }

        //— Mutators ——
        void set_handle(const unsigned int h) { handle_ = h; }
        void set_one_half_edge(const std::shared_ptr<halfedge>& he) { one_half_edge_ = he; }

    private:
        std::weak_ptr<vertex> v1_, v2_, v3_;
        unsigned int handle_ = std::numeric_limits<unsigned>::max();
        std::weak_ptr<halfedge> one_half_edge_;
    };
} // namespace HalfMesh
