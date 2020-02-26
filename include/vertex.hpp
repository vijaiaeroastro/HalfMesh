#pragma once

#include <general.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>

namespace HalfMesh {
    // A vertex structure
    class Vertex {
    public:
        Vertex(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};

        ~Vertex() {};

    public:
        void set_handle(unsigned int _i) {
            vertex_handle = _i;
        }

        unsigned int handle() {
            if (this == NULL_VERTEX) {
                return std::numeric_limits<unsigned int>::max();
            }
            return vertex_handle;
        }

        void set_x(double _x) {
            x = _x;
        }

        void set_y(double _y) {
            y = _y;
        }

        void set_z(double _z) {
            z = _z;
        }

        double get_x() {
            return x;
        }

        double get_y() {
            return y;
        }

        double get_z() {
            return z;
        }

        void add_incoming_half_edge(unsigned int _i) {
            incoming_half_edges.insert(_i);
        }

        void add_outgoing_half_edge(unsigned int _i) {
            outgoing_half_edges.insert(_i);
        }

        std::unordered_set<unsigned int> get_incoming_half_edges() {
            return incoming_half_edges;
        }

        std::unordered_set<unsigned int> get_outgoing_half_edges() {
            return outgoing_half_edges;
        }

    private:
        double x, y, z;
        unsigned int vertex_handle;
        std::unordered_set<unsigned int> incoming_half_edges;
        std::unordered_set<unsigned int> outgoing_half_edges;
    };
}