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
                return v2;
            }

            void set_handle(unsigned int _i)
            {
                half_edge_handle = _i;
            }

            unsigned int handle()
            {
                if(this == NULL_HALF_EDGE)
                {
                    return std::numeric_limits<unsigned int>::max();
                }
                return half_edge_handle;
            }

            void set_parent_edge(unsigned int _i)
            {
                parent_edge_handle = _i;
            }

            unsigned int get_parent_edge()
            {
                return parent_edge_handle;
            }

            void set_parent_face(unsigned int _i)
            {
                parent_face_handle = _i;
            }

            unsigned int get_parent_face()
            {
                return parent_face_handle;
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


        private:
            Vertex *v1, *v2;
            unsigned int half_edge_handle;
            unsigned int parent_edge_handle;
            unsigned int parent_face_handle;
            unsigned int opposing_half_edge = std::numeric_limits<unsigned int>::max();
            bool boundary_half_edge;
    };
}
