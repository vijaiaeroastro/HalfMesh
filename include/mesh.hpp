#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>

namespace HalfMesh
{
    class Mesh {
    public:
        Vertex *add_vertex(Vertex *new_vertex) {
            unsigned int return_handle;
            if (vertex_to_vertex_handle_map.find(new_vertex) == vertex_to_vertex_handle_map.end()) {
                unsigned int new_vertex_handle;
                if (all_vertices.size() == 0) {
                    new_vertex_handle = 0;
                } else {
                    new_vertex_handle = all_vertices.size();
                }
                new_vertex->id = new_vertex_handle;
                all_vertices.push_back(new_vertex);
                vertex_to_vertex_handle_map[new_vertex] = new_vertex_handle;
                return_handle = new_vertex_handle;
                vertex_handle_to_vertex_map[new_vertex_handle] = new_vertex;
                std::cout << "---> New Vertex added with handle : " << return_handle << std::endl;
            } else {
                return_handle = vertex_to_vertex_handle_map[new_vertex];
                std::cout << "---> A Vertex exists with handle : " << return_handle << std::endl;
            }
            return vertex_handle_to_vertex_map[return_handle];
        }

        HalfEdge *add_half_edge(Vertex *v1, Vertex *v2, Face *f1) {
            unsigned int half_edge_handle;
            if (vertex_to_half_edge_map.find(std::make_tuple(v1->id, v2->id)) == vertex_to_half_edge_map.end()) {
                if (all_half_edges.size() == 0) {
                    half_edge_handle = 0;
                } else {
                    half_edge_handle = all_half_edges.size();
                }
                HalfEdge *new_half_edge = new HalfEdge(v1, v2);
                new_half_edge->half_edge_handle = half_edge_handle;
                new_half_edge->parent_face_handle = f1->face_handle;
                if (vertex_to_half_edge_map.find(std::make_tuple(v2->id, v1->id)) != vertex_to_half_edge_map.end()) {
                    new_half_edge->opposing_half_edge = vertex_to_half_edge_map[std::make_tuple(v2->id,
                                                                                                v1->id)]->half_edge_handle;
                }
                v1->outgoing_half_edges.insert(half_edge_handle);
                v2->incoming_half_edges.insert(half_edge_handle);
                all_half_edges.push_back(new_half_edge);
                half_edge_handle_to_half_edge_map[half_edge_handle] = new_half_edge;
                face_to_one_half_edge_map[f1] = new_half_edge;
                std::cout << "--->--->---> New Half Edge added with handle : " << half_edge_handle << std::endl;
            }
            else
            {
                half_edge_handle = vertex_to_half_edge_map[std::make_tuple(v1->id, v2->id)]->half_edge_handle;
                std::cout << "--->--->---> A Half Edge exists with handle : " << half_edge_handle << std::endl;
            }
            return half_edge_handle_to_half_edge_map[half_edge_handle];
        }

        Edge *add_edge(Vertex *v1, Vertex *v2, Face *f1) {
            unsigned int edge_handle;
            if (vertices_to_edge_handle_map.find(std::make_tuple(v1->id, v2->id)) ==
                vertices_to_edge_handle_map.end()) {
                if (all_edges.size() == 0) {
                    edge_handle = 0;
                } else {
                    edge_handle = all_edges.size();
                }
                Edge *new_edge = new Edge(v1, v2);
                new_edge->edge_handle = edge_handle;
                all_edges.push_back(new_edge);
                vertices_to_edge_handle_map[std::make_tuple(v1->id, v2->id)] = edge_handle;
                std::cout << "--->---> New Edge added with handle : " << edge_handle << std::endl;
                edge_handle_to_edge_map[edge_handle] = new_edge;
                HalfEdge *he = add_half_edge(v1, v2, f1);
                he->parent_edge_handle = edge_handle;
                edge_to_one_half_edge_map[new_edge] = he;
            } else {
                edge_handle = vertices_to_edge_handle_map[std::make_tuple(v1->id, v2->id)];
                std::cout << "--->--->An Edge exists with handle : " << edge_handle << std::endl;
            }
            return edge_handle_to_edge_map[edge_handle];
        }

        Face *add_face(Vertex *v1, Vertex *v2, Vertex *v3) {
            unsigned int face_handle;
            if (vertices_to_face_handle_map.find(std::make_tuple(v1->id, v2->id, v3->id)) ==
                vertices_to_face_handle_map.end()) {
                if (all_faces.size() == 0) {
                    face_handle = 0;
                } else {
                    face_handle = all_faces.size();
                }
                Face *new_face = new Face(v1, v2, v3);
                new_face->face_handle = face_handle;
                all_faces.push_back(new_face);
                vertices_to_face_handle_map[std::make_tuple(v1->id, v2->id, v3->id)] = face_handle;
                std::cout << "---> New Face added with handle : " << face_handle << std::endl;
                face_handle_to_face_map[face_handle] = new_face;
                Edge *e1 = add_edge(v1, v2, new_face);
                Edge *e2 = add_edge(v2, v3, new_face);
                Edge *e3 = add_edge(v3, v1, new_face);
            } else {
                face_handle = vertices_to_face_handle_map[std::make_tuple(v1->id, v2->id, v3->id)];
                std::cout << "---> A Face exists with handle : " << face_handle << std::endl;
            }
            return face_handle_to_face_map[face_handle];
        }


    private:
        void establish_connectivity() {
            for(auto iter = face_to_one_half_edge_map.begin(); iter != face_to_one_half_edge_map.end(); ++iter)
            {
                Face *face = iter->first;
                HalfEdge *halfEdge = iter->second;
                face->one_half_edge = halfEdge;
            }
            for(auto iter = edge_to_one_half_edge_map.begin(); iter != edge_to_one_half_edge_map.end(); ++iter)
            {
                Edge *edge = iter->first;
                HalfEdge *halfEdge = iter->second;
                edge->one_half_edge = halfEdge;
            }
            for(unsigned int i = 0; i < all_half_edges.size(); ++i)
            {
                HalfEdge *edge = all_half_edges.at(i);
                if(edge->opposing_half_edge  == std::numeric_limits<unsigned int>::max())
                {
                    edge->boundary_half_edge = true;
                }
            }
            for(unsigned int i = 0; i < all_edges.size(); ++i)
            {
                Edge *edge = all_edges.at(i);
                HalfEdge *halfEdge = edge->one_half_edge;
                if(halfEdge->boundary_half_edge)
                {
                    edge->boundary_edge = true;
                }
            }
        }

    public:
        void complete_mesh() {
            establish_connectivity();
        }

    public:
        std::vector<Face *> get_faces() {
            return all_faces;
        }

        std::vector<Edge *> get_edges() {
            return all_edges;
        }

        std::vector<HalfEdge *> get_half_edges() {
            return all_half_edges;
        }

        std::vector<Vertex *> get_vertices() {
            return all_vertices;
        }

    public:
        Face *get_face(unsigned int i) {
            Face *new_face = NULL;
            if (face_handle_to_face_map.find(i) != face_handle_to_face_map.end()) {
                return face_handle_to_face_map[i];
            } else {
                return NULL;
            }
        }

        Edge *get_edge(unsigned int i)
        {
            Edge* new_edge = NULL;
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
            Vertex* new_vertex = NULL;
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
            HalfEdge* new_half_edge = NULL;
            if(half_edge_handle_to_half_edge_map.find(i) != half_edge_handle_to_half_edge_map.end())
            {
                return half_edge_handle_to_half_edge_map[i];
            }
            else
            {
                return new_half_edge;
            }
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