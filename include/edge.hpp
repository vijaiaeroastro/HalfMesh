#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <half_edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    // An edge structure
    class Edge
    {
        public:
            Edge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
            ~Edge(){};

        public:
            Vertex* get_vertex_one()
            {
                return v1;
            }

            Vertex* get_vertex_two()
            {
                return v2;
            }

            void set_vertex_one(Vertex *_v1)
            {
                v1 = _v1;
            }

            void set_vertex_two(Vertex *_v2)
            {
                v2 = _v2;
            }

            void set_handle(unsigned int _i)
            {
                edge_handle = _i;
            }

            unsigned int handle()
            {
                return edge_handle;
            }

            void set_boundary(bool boundariness)
            {
                boundary_edge = boundariness;
            }

            bool is_boundary()
            {
                return boundary_edge;
            }

            void set_one_half_edge(HalfEdge* _half_edge)
            {
                one_half_edge = _half_edge;
            }

            HalfEdge* get_one_half_edge()
            {
                return one_half_edge;
            }

        private:
            Vertex *v1, *v2;
            unsigned int edge_handle;
            HalfEdge *one_half_edge = NULL_HALF_EDGE;
            bool boundary_edge = false;

    };
}
