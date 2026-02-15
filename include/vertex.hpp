#pragma once

#include <common.hpp>
#include <limits>
#include <memory>
#include <vector>

namespace halfMesh {
    class vertex {
    public:
        vertex(double x, double y, double z)
            : x_(x), y_(y), z_(z) {
        }

        ~vertex() = default;

        //— Accessors ——
        unsigned int get_handle() const { return handle_; }
        double get_x() const { return x_; }
        double get_y() const { return y_; }
        double get_z() const { return z_; }
        Vec3 get_position() const {
            return {x_, y_, z_};
        }

        std::vector<std::shared_ptr<halfedge> > get_incoming_half_edges() const {
            std::vector<std::shared_ptr<halfedge> > out;
            out.reserve(incoming_.size());
            for (auto &wp: incoming_) {
                if (auto sp = wp.lock()) out.push_back(sp);
            }
            return out;
        }

        std::vector<std::shared_ptr<halfedge> > get_outgoing_half_edges() const {
            std::vector<std::shared_ptr<halfedge> > out;
            out.reserve(outgoing_.size());
            for (auto &wp: outgoing_) {
                if (auto sp = wp.lock()) out.push_back(sp);
            }
            return out;
        }

        //— Mutators ——
        void set_handle(const unsigned int h) { handle_ = h; }
        void set_x(const double x) { x_ = x; }
        void set_y(const double y) { y_ = y; }
        void set_z(const double z) { z_ = z; }

        void add_incoming_half_edge(const std::shared_ptr<halfedge> &he) {
            incoming_.push_back(he);
        }

        void add_outgoing_half_edge(const std::shared_ptr<halfedge> &he) {
            outgoing_.push_back(he);
        }

    private:
        double x_, y_, z_;
        unsigned int handle_ = std::numeric_limits<unsigned>::max();
        std::vector<std::weak_ptr<halfedge> > incoming_;
        std::vector<std::weak_ptr<halfedge> > outgoing_;
    };
} // namespace HalfMesh
