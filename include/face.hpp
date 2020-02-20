#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>

namespace HalfMesh
{
    // A face structure
    class Face
    {
        public:
            Face(Vertex *_v1, Vertex *_v2, Vertex *_v3):v1(_v1), v2(_v2), v3(_v3){};
            ~Face(){};

        public:
            void set_handle(unsigned int _i)
            {
                face_handle = _i;
            }

            unsigned int handle()
            {
                return face_handle;
            }

            void set_one_half_edge(HalfEdge* half_edge)
            {
                one_half_edge = half_edge;
            }

            HalfEdge* get_one_half_edge()
            {
                return one_half_edge;
            }

            void swap_vertex(Vertex *_v1=NULL_VERTEX, Vertex *_v2=NULL_VERTEX, Vertex *_v3=NULL_VERTEX)
            {
                if (_v1 != NULL_VERTEX)
                {
                    v1 = _v1;
                }
                if (_v2 != NULL_VERTEX)
                {
                    v2 = _v2;
                }
                if(_v3 != NULL_VERTEX)
                {
                    v3 = _v3;
                }
            }

            Vertex* get_vertex_one()
            {
                return v1;
            }

            Vertex* get_vertex_two()
            {
                return v2;
            }

            Vertex* get_vertex_three()
            {
                return v3;
            }

            std::tuple< Vertex*, Vertex*, Vertex* > get_vertices()
            {
                return std::make_tuple(v1,v2,v3);
            }

            std::tuple< double, double, double > get_normal()
            {
                double x = 0.0;
                double y = 0.0;
                double z = 0.0;
                return std::make_tuple(x,y,z);
            }

        private:
            Vertex *v1, *v2, *v3;
            unsigned int face_handle;
            HalfEdge *one_half_edge = NULL_HALF_EDGE;
    };

}