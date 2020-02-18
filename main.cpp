#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <unordered_map>

typedef std::tuple<unsigned int, unsigned int, unsigned int> key_type_t;

struct key_hash : public std::unary_function<key_type_t, std::size_t>
{
    std::size_t operator()(const key_type_t& k) const
    {
        return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
    }
};

struct key_equal : public std::binary_function<key_type_t, key_type_t, bool>
{
    bool operator()(const key_type_t& v0, const key_type_t& v1) const
    {
        return (
                std::get<0>(v0) == std::get<0>(v1) &&
                std::get<1>(v0) == std::get<1>(v1) &&
                std::get<2>(v0) == std::get<2>(v1)
        );
    }
};

typedef std::unordered_map<const key_type_t, unsigned int,key_hash,key_equal> special_map;

struct Vertex
{
    Vertex(double _x1, double _x2, double _x3):x1(_x1), x2(_x2), x3(_x3){};
    double x1, x2, x3;
    unsigned int id;
};

struct Edge
{
    unsigned int v1, v2;
    unsigned int edge_handle;
};

struct Face
{
    Face(Vertex *_v1, Vertex *_v2, Vertex *_v3):v1(_v1), v2(_v2), v3(_v3){};
    Vertex *v1, *v2, *v3;
    unsigned int face_handle;
};

struct HalfEdge
{
    unsigned int from_vertex, to_vertex;
    unsigned int parent_face_handle;
    unsigned int opposing_half_edge;
    unsigned int next_half_edge;
};


struct Mesh
{
    std::vector< Face* > all_faces;
    std::vector< Edge* > all_edges;
    std::vector< HalfEdge* > all_half_edges;
    std::vector< Vertex* > all_vertices;
    std::map< Vertex*, unsigned int > vertex_to_vertex_handle_map;
    special_map vertices_to_face_handle_map;

    unsigned int add_vertex(Vertex *new_vertex)
    {
        unsigned int return_handle;
        if(vertex_to_vertex_handle_map.find(new_vertex) == vertex_to_vertex_handle_map.end())
        {
            unsigned int new_vertex_handle;
            if(all_vertices.size() == 0)
            {
                new_vertex_handle = 0;
            }
            else
            {
                new_vertex_handle = all_vertices.size();
            }
            new_vertex->id = new_vertex_handle;
            all_vertices.push_back(new_vertex);
            vertex_to_vertex_handle_map[new_vertex] = new_vertex_handle;
            return_handle = new_vertex_handle;
        }
        else
        {
            return_handle = vertex_to_vertex_handle_map[new_vertex];
        }
        std::cout << "New Vertex added with handle : " << return_handle << std::endl;
        return return_handle;
    }

    unsigned int add_face(Vertex *v1, Vertex *v2, Vertex* v3)
    {
        unsigned int face_handle;
        if (vertices_to_face_handle_map.find(std::make_tuple(v1->id, v2->id, v3->id)) == vertices_to_face_handle_map.end())
        {
            if(all_faces.size() == 0)
            {
                face_handle = 0;
            }
            else
            {
                face_handle = all_faces.size();
            }
            Face *new_face = new Face(v1,v2,v3);
            new_face->face_handle = face_handle;
            all_faces.push_back(new_face);
            vertices_to_face_handle_map[std::make_tuple(v1->id, v2->id, v3->id)] = face_handle;
        }
        else
        {
            face_handle = vertices_to_face_handle_map[std::make_tuple(v1->id, v2->id, v3->id)];
        }
        std::cout << "New Face added with handle : " << face_handle << std::endl;
        return face_handle;
    }

};

int main()
{
    Mesh *new_mesh = new Mesh;
    Vertex *v1 = new Vertex(0.0,0.0,0.0);
    Vertex *v2 = new Vertex(1.0,0.0,0.0);
    Vertex *v3 = new Vertex(0.0,0.5,0.0);
    Vertex *v4 = new Vertex(1.5,0.5,0.0);
    new_mesh->add_vertex(v1);
    new_mesh->add_vertex(v2);
    new_mesh->add_vertex(v3);
    new_mesh->add_vertex(v4);
    new_mesh->add_face(v1,v2,v3);
    new_mesh->add_face(v2,v4,v3);
    return 0;
}
