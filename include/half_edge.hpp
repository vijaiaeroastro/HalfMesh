#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    // A Half edge structure
    class HalfEdge
    {
        public:
            HalfEdge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
            ~HalfEdge(){};

        public:
            void set_vertex_one(Vertex* _v1)
            {
                v1 = _v1;
            }

            void set_vertex_two(Vertex* _v2)
            {
                v2 = _v2;
            }

            Vertex* get_vertex_one()
            {
                return v1;
            }

            Vertex* get_vertex_two()
            {
                return v1;
            }

            void set_handle(unsigned int _i)
            {
                half_edge_handle = _i;
            }

            unsigned int handle()
            {
                return half_edge_handle;
            }

            void set_parent_edge(unsigned int _i)
            {
                parent_edge_handle = _i;
            }

            void set_parent_face(unsigned int _i)
            {
                parent_face_handle = _i;
            }

            void set_boundary(bool _boundariness)
            {
                boundary_half_edge = _boundariness;
            }

            bool is_boundary()
            {
                return boundary_half_edge;
            }

            void set_opposing_half_edge(unsigned int _i)
            {
                opposing_half_edge = _i;
            }

            unsigned int get_opposing_half_edge()
            {
                return opposing_half_edge;
            }

            void set_next_half_edge(unsigned int _i)
            {
                next_half_edge = _i;
            }

            unsigned int get_next_half_edge()
            {
                return next_half_edge;
            }


        private:
            Vertex *v1, *v2;
            unsigned int half_edge_handle;
            unsigned int parent_edge_handle;
            unsigned int parent_face_handle;
            unsigned int opposing_half_edge = std::numeric_limits<unsigned int>::max();
            unsigned int next_half_edge;
            bool boundary_half_edge;
    };
}
