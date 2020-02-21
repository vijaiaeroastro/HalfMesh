#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>
#include <property.hpp>

namespace HalfMesh
{
    class Mesh {
    public:
        Vertex *add_vertex(Vertex *new_vertex)
        {
            unsigned int return_handle;
            if (vertex_to_vertex_handle_map.find(new_vertex) == vertex_to_vertex_handle_map.end())
            {
                unsigned int new_vertex_handle;
                if (all_vertices.size() == 0)
                {
                    new_vertex_handle = 0;
                }
                else
                {
                    new_vertex_handle = all_vertices.size();
                }
                new_vertex->set_handle(new_vertex_handle);
                all_vertices.push_back(new_vertex);
                vertex_to_vertex_handle_map[new_vertex] = new_vertex_handle;
                return_handle = new_vertex_handle;
                vertex_handle_to_vertex_map[new_vertex_handle] = new_vertex;
#ifndef NDEBUG
                std::cout << "---> New Vertex added with handle : " << return_handle << std::endl;
#endif
            }
            else
            {
                return_handle = vertex_to_vertex_handle_map[new_vertex];
#ifndef NDEBUG
                std::cout << "---> A Vertex exists with handle : " << return_handle << std::endl;
#endif
            }
            return vertex_handle_to_vertex_map[return_handle];
        }

        HalfEdge *add_half_edge(Vertex *v1, Vertex *v2, Face *f1)
        {
            unsigned int half_edge_handle;
            if (vertex_to_half_edge_map.find(std::make_tuple(v1->handle(), v2->handle())) == vertex_to_half_edge_map.end())
            {
                if (all_half_edges.size() == 0)
                {
                    half_edge_handle = 0;
                }
                else
                {
                    half_edge_handle = all_half_edges.size();
                }
                HalfEdge *new_half_edge = new HalfEdge(v1, v2);
                new_half_edge->set_handle(half_edge_handle);
                new_half_edge->set_parent_face(f1->handle());
                if (vertex_to_half_edge_map.find(std::make_tuple(v2->handle(), v1->handle())) != vertex_to_half_edge_map.end())
                {
                    unsigned int existing_half_edge = vertex_to_half_edge_map[std::make_tuple(v2->handle(), v1->handle())]->handle();
                    new_half_edge->set_opposing_half_edge(existing_half_edge);
                    get_half_edge(existing_half_edge)->set_opposing_half_edge(new_half_edge->handle());
                }
                v1->add_outgoing_half_edge(half_edge_handle);
                v2->add_incoming_half_edge(half_edge_handle);
                all_half_edges.push_back(new_half_edge);
                half_edge_handle_to_half_edge_map[half_edge_handle] = new_half_edge;
                face_to_one_half_edge_map[f1] = new_half_edge;
                vertex_to_half_edge_map[std::make_tuple(v1->handle(), v2->handle())] = new_half_edge;
#ifndef NDEBUG
                std::cout << "--->--->---> New Half Edge added with handle : " << half_edge_handle << std::endl;
#endif
            }
            else
            {
                half_edge_handle = vertex_to_half_edge_map[std::make_tuple(v1->handle(), v2->handle())]->handle();
#ifndef NDEBUG
                std::cout << "--->--->---> A Half Edge exists with handle : " << half_edge_handle << std::endl;
#endif
            }
            return half_edge_handle_to_half_edge_map[half_edge_handle];
        }

        Edge *add_edge(Vertex *v1, Vertex *v2, Face *f1)
        {
            unsigned int edge_handle;
            if (vertices_to_edge_handle_map.find(std::make_tuple(v1->handle(), v2->handle())) ==
                vertices_to_edge_handle_map.end())
            {
                if(vertices_to_edge_handle_map.find(std::make_tuple(v2->handle(), v1->handle())) == vertices_to_edge_handle_map.end())
                {
                    if (all_edges.size() == 0)
                    {
                        edge_handle = 0;
                    }
                    else
                    {
                        edge_handle = all_edges.size();
                    }
                    Edge *new_edge = new Edge(v1, v2);
                    new_edge->set_handle(edge_handle);
                    all_edges.push_back(new_edge);
                    vertices_to_edge_handle_map[std::make_tuple(v1->handle(), v2->handle())] = edge_handle;
#ifndef NDEBUG
                    std::cout << "--->---> New Edge added with handle : " << edge_handle << std::endl;
#endif
                    edge_handle_to_edge_map[edge_handle] = new_edge;
                    HalfEdge *he = add_half_edge(v1, v2, f1);
                    he->set_parent_edge(edge_handle);
                    edge_to_one_half_edge_map[new_edge] = he;
                }
                else
                {
                    edge_handle = vertices_to_edge_handle_map[std::make_tuple(v2->handle(), v1->handle())];
#ifndef NDEBUG
                    std::cout << "--->---> An Edge exists with handle : " << edge_handle << std::endl;
#endif
                    Edge *existing_edge = edge_handle_to_edge_map[edge_handle];
                    HalfEdge *he = add_half_edge(v1, v2, f1);
                    he->set_parent_edge(edge_handle);
                    edge_to_one_half_edge_map[existing_edge] = he;
                }
            }
            else
            {
                edge_handle = vertices_to_edge_handle_map[std::make_tuple(v1->handle(), v2->handle())];
#ifndef NDEBUG
                std::cout << "--->---> An Edge exists with handle : " << edge_handle << std::endl;
#endif
            }
            return edge_handle_to_edge_map[edge_handle];
        }

        Face *add_face(Vertex *v1, Vertex *v2, Vertex *v3)
        {
            unsigned int face_handle;
            if (vertices_to_face_handle_map.find(std::make_tuple(v1->handle(), v2->handle(), v3->handle())) ==
                vertices_to_face_handle_map.end())
            {
                if (all_faces.size() == 0)
                {
                    face_handle = 0;
                }
                else
                {
                    face_handle = all_faces.size();
                }
                Face *new_face = new Face(v1, v2, v3);
                new_face->set_handle(face_handle);
                all_faces.push_back(new_face);
                vertices_to_face_handle_map[std::make_tuple(v1->handle(), v2->handle(), v3->handle())] = face_handle;
#ifndef NDEBUG
                std::cout << "---> New Face added with handle : " << face_handle << std::endl;
#endif
                face_handle_to_face_map[face_handle] = new_face;
                Edge *e1 = add_edge(v1, v2, new_face);
                Edge *e2 = add_edge(v2, v3, new_face);
                Edge *e3 = add_edge(v3, v1, new_face);
            }
            else
            {
                face_handle = vertices_to_face_handle_map[std::make_tuple(v1->handle(), v2->handle(), v3->handle())];
#ifndef NDEBUG
                std::cout << "---> A Face exists with handle : " << face_handle << std::endl;
#endif
            }
            return face_handle_to_face_map[face_handle];
        }


    private:
        void establish_connectivity()
        {
            for(auto iter = face_to_one_half_edge_map.begin(); iter != face_to_one_half_edge_map.end(); ++iter)
            {
                Face *face = iter->first;
                HalfEdge *halfEdge = iter->second;
                face->set_one_half_edge(halfEdge);
            }
            for(auto iter = edge_to_one_half_edge_map.begin(); iter != edge_to_one_half_edge_map.end(); ++iter)
            {
                Edge *edge = iter->first;
                HalfEdge *halfEdge = iter->second;
                edge->set_one_half_edge(halfEdge);
            }
            for(unsigned int i = 0; i < all_half_edges.size(); ++i)
            {
                HalfEdge *half_edge = all_half_edges.at(i);
                if(half_edge->get_opposing_half_edge()  == std::numeric_limits<unsigned int>::max())
                {
                    half_edge->set_boundary(true);
                }
            }
            for(unsigned int i = 0; i < all_edges.size(); ++i)
            {
                Edge *edge = all_edges.at(i);
                HalfEdge *halfEdge = edge->get_one_half_edge();

                if(halfEdge->is_boundary())
                {
                    edge->set_boundary(true);
                }
            }
        }

    public:
        void complete_mesh()
        {
            if(all_faces.size() > 0)
            {
                establish_connectivity();
            }
        }

    public:
        std::vector<Face *> get_faces()
        {
            return all_faces;
        }

        std::vector<Edge *> get_edges()
        {
            return all_edges;
        }

        std::vector<HalfEdge *> get_half_edges()
        {
            return all_half_edges;
        }

        std::vector<Vertex *> get_vertices()
        {
            return all_vertices;
        }

    public:
        Face* get_face(unsigned int i)
        {
            Face *new_face = NULL_FACE;
            if (face_handle_to_face_map.find(i) != face_handle_to_face_map.end())
            {
                return face_handle_to_face_map[i];
            }
            else
            {
                return new_face;
            }
        }

        Edge* get_edge(unsigned int i)
        {
            Edge* new_edge = NULL_EDGE;
            if(edge_handle_to_edge_map.find(i) != edge_handle_to_edge_map.end())
            {
                return edge_handle_to_edge_map[i];
            }
            else
            {
                return new_edge;
            }
        }

        Vertex* get_vertex(unsigned int i)
        {
            Vertex* new_vertex = NULL_VERTEX;
            if(vertex_handle_to_vertex_map.find(i) != vertex_handle_to_vertex_map.end())
            {
                return vertex_handle_to_vertex_map[i];
            }
            else
            {
                return new_vertex;
            }
        }

        HalfEdge* get_half_edge(unsigned int i)
        {
            HalfEdge* new_half_edge = NULL_HALF_EDGE;
            if(half_edge_handle_to_half_edge_map.find(i) != half_edge_handle_to_half_edge_map.end())
            {
                return half_edge_handle_to_half_edge_map[i];
            }
            else
            {
                return new_half_edge;
            }
        }

        public:
            HalfEdge* get_next_half_edge(HalfEdge* input_half_edge, Face* input_face)
            {
                HalfEdge* return_edge = NULL_HALF_EDGE;
                std::unordered_set< unsigned int > outgoing_half_edges = input_half_edge->get_vertex_two()->get_outgoing_half_edges();
                for(auto iter = outgoing_half_edges.begin(); iter != outgoing_half_edges.end(); ++iter)
                {
                    HalfEdge* current_outgoing_half_edge = half_edge_handle_to_half_edge_map[*iter];
                    if(current_outgoing_half_edge == input_half_edge)
                    {
                        continue;
                    }
                    if(current_outgoing_half_edge->get_parent_face() == input_face->handle())
                    {
                        return_edge = current_outgoing_half_edge;
                    }
                }
                return return_edge;
            }

            HalfEdge* get_previous_half_edge(HalfEdge* input_half_edge, Face* input_face)
            {
                HalfEdge* return_edge = NULL_HALF_EDGE;
                std::unordered_set< unsigned int > incoming_half_edges = input_half_edge->get_vertex_one()->get_incoming_half_edges();
                for(auto iter = incoming_half_edges.begin(); iter != incoming_half_edges.end(); ++iter)
                {
                    HalfEdge* current_incoming_half_edge = half_edge_handle_to_half_edge_map[*iter];
                    if(current_incoming_half_edge == input_half_edge)
                    {
                        continue;
                    }
                    if(current_incoming_half_edge->get_parent_face() == input_face->handle())
                    {
                        return_edge = current_incoming_half_edge;
                    }
                }
                return return_edge;
            }

        private:
            typedef std::unordered_map<const twin_type_t, unsigned int, twin_key_hash, twin_key_equal> special_map_twin;
            typedef std::unordered_map<const twin_type_t, HalfEdge*, twin_key_hash, twin_key_equal> twin_map_he_special;
            std::vector< Face* > all_faces;
            std::vector< Edge* > all_edges;
            std::vector< HalfEdge* > all_half_edges;
            std::vector< Vertex* > all_vertices;
            std::unordered_map< Vertex*, unsigned int > vertex_to_vertex_handle_map;
            special_map_three vertices_to_face_handle_map;
            special_map_twin vertices_to_edge_handle_map;
            std::unordered_map< unsigned int, Vertex* > vertex_handle_to_vertex_map;
            std::unordered_map< unsigned int, Edge* > edge_handle_to_edge_map;
            std::unordered_map< unsigned int, HalfEdge* > half_edge_handle_to_half_edge_map;
            std::unordered_map< unsigned int, Face* > face_handle_to_face_map;
            std::unordered_map< Face*, HalfEdge* > face_to_one_half_edge_map;
            std::unordered_map< Edge*, HalfEdge* > edge_to_one_half_edge_map;
            twin_map_he_special vertex_to_half_edge_map;

    };   
}