#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <unordered_map>

// Forward declarations
struct Vertex;
struct Edge;
struct HalfEdge;
struct Face;

// Triple key tuple hash map

typedef std::tuple<unsigned int, unsigned int, unsigned int> three_type_t;

struct three_key_hash : public std::unary_function<three_type_t, std::size_t>
{
    std::size_t operator()(const three_type_t& k) const
    {
        return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
    }
};

struct three_key_equal : public std::binary_function<three_type_t, three_type_t, bool>
{
    bool operator()(const three_type_t& v0, const three_type_t& v1) const
    {
        return (
                std::get<0>(v0) == std::get<0>(v1) &&
                std::get<1>(v0) == std::get<1>(v1) &&
                std::get<2>(v0) == std::get<2>(v1)
        );
    }
};

typedef std::unordered_map<const three_type_t, unsigned int,three_key_hash,three_key_equal> special_map_three;


// Double key tuple hash map
typedef std::tuple<unsigned int, unsigned int> twin_type_t;

struct twin_key_hash : public std::unary_function<twin_type_t, std::size_t>
{
    std::size_t operator()(const twin_type_t& k) const
    {
        return std::get<0>(k) ^ std::get<1>(k);
    }
};

struct twin_key_equal : public std::binary_function<twin_type_t, twin_type_t, bool>
{
    bool operator()(const twin_type_t& v0, const twin_type_t& v1) const
    {
        return (
                std::get<0>(v0) == std::get<0>(v1) &&
                std::get<1>(v0) == std::get<1>(v1)
        );
    }
};

typedef std::unordered_map<const twin_type_t, unsigned int, twin_key_hash, twin_key_equal> special_map_twin;
typedef std::unordered_map<const twin_type_t, HalfEdge*, twin_key_hash, twin_key_equal> twin_map_he_special;

// A vertex structure
struct Vertex
{
    Vertex(double _x1, double _x2, double _x3):x1(_x1), x2(_x2), x3(_x3){};
    double x1, x2, x3;
    unsigned int id;
};

// An edge structure
struct Edge
{
    Edge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
    Vertex *v1, *v2;
    unsigned int edge_handle;
    std::pair< HalfEdge*, HalfEdge* > half_edges;

};

// A face structure
struct Face
{
    Face(Vertex *_v1, Vertex *_v2, Vertex *_v3):v1(_v1), v2(_v2), v3(_v3){};
    Vertex *v1, *v2, *v3;
    unsigned int face_handle;
    HalfEdge *one_half_edge = NULL;
};

// A Half edge structure
struct HalfEdge
{
    HalfEdge(Vertex *_v1, Vertex *_v2):v1(_v1), v2(_v2){};
    Vertex *v1, *v2;
    unsigned int half_edge_handle;
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
    std::unordered_map< Vertex*, unsigned int > vertex_to_vertex_handle_map;
    special_map_three vertices_to_face_handle_map;
    special_map_twin vertices_to_edge_handle_map;
    std::unordered_map< unsigned int, unsigned int > edge_to_half_edge_map;
    std::unordered_map< unsigned int, unsigned int > half_edge_edge_map; // Replace these with bidirectional maps
    std::unordered_map< unsigned int, unsigned int > half_edge_to_face_map;
    std::unordered_map< unsigned int, Vertex* > vertex_handle_to_vertex_map;
    std::unordered_map< unsigned int, Edge* > edge_handle_to_edge_map;
    std::unordered_map< unsigned int, HalfEdge* > half_edge_handle_to_half_edge_map;
    std::unordered_map< unsigned int, Face* > face_handle_to_face_map;
    twin_map_he_special vertex_to_half_edge_map;

    void add_vertex(Vertex *new_vertex)
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
            vertex_handle_to_vertex_map[new_vertex_handle] = new_vertex;
            std::cout << "---> New Vertex added with handle : " << return_handle << std::endl;
        }
        else
        {
            return_handle = vertex_to_vertex_handle_map[new_vertex];
            std::cout << "---> A Vertex exists with handle : " << return_handle << std::endl;
        }
    }

    void add_half_edge(Vertex *v1, Vertex *v2, Face *f1)
    {
        unsigned int half_edge_handle;
        if(vertex_to_half_edge_map.find(std::make_tuple(v1->id, v2->id)) == vertex_to_half_edge_map.end())
        {
            if(all_half_edges.size() == 0)
            {
                half_edge_handle = 0;
            }
            else
            {
                half_edge_handle = all_half_edges.size();
            }
            HalfEdge* new_half_edge = new HalfEdge(v1,v2);
            new_half_edge->half_edge_handle = half_edge_handle;
            new_half_edge->parent_face_handle = f1->face_handle;
            if(vertex_to_half_edge_map.find(std::make_tuple(v2->id, v1->id)) != vertex_to_half_edge_map.end())
            {
                new_half_edge->opposing_half_edge = vertex_to_half_edge_map[std::make_tuple(v2->id,v1->id)]->half_edge_handle;
            }
            all_half_edges.push_back(new_half_edge);
            half_edge_handle_to_half_edge_map[half_edge_handle] = new_half_edge;
            std::cout << "--->--->---> New Half Edge added with handle : " << half_edge_handle << std::endl;
        }
        else
        {
            half_edge_handle = vertex_to_half_edge_map[std::make_tuple(v1->id, v2->id)]->half_edge_handle;
            std::cout << "--->--->---> A Half Edge exists with handle : " << half_edge_handle << std::endl;
        }
    }

    void add_edge(Vertex *v1, Vertex *v2, Face* f1)
    {
        unsigned int edge_handle;
        if(vertices_to_edge_handle_map.find(std::make_tuple(v1->id, v2->id)) == vertices_to_edge_handle_map.end())
        {
            if(all_edges.size() == 0)
            {
                edge_handle = 0;
            }
            else
            {
                edge_handle = all_edges.size();
            }
            Edge* new_edge = new Edge(v1, v2);
            new_edge->edge_handle = edge_handle;
            all_edges.push_back(new_edge);
            vertices_to_edge_handle_map[std::make_tuple(v1->id, v2->id)] = edge_handle;
            std::cout << "--->---> New Edge added with handle : " << edge_handle << std::endl;
            edge_handle_to_edge_map[edge_handle] = new_edge;
            add_half_edge(v1, v2, f1);
        }
        else
        {
            edge_handle = vertices_to_edge_handle_map[std::make_tuple(v1->id, v2->id)];
            std::cout << "--->--->An Edge exists with handle : " << edge_handle << std::endl;
        }
    }

    void add_face(Vertex *v1, Vertex *v2, Vertex* v3)
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
            std::cout << "---> New Face added with handle : " << face_handle << std::endl;
            face_handle_to_face_map[face_handle] = new_face;
            add_edge(v1,v2, new_face);
            add_edge(v2,v3, new_face);
            add_edge(v3,v2, new_face);
        }
        else
        {
            face_handle = vertices_to_face_handle_map[std::make_tuple(v1->id, v2->id, v3->id)];
            std::cout << "---> A Face exists with handle : " << face_handle << std::endl;
        }
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
    // Try adding duplicate vertex here
    new_mesh->add_vertex(v2);
    new_mesh->add_face(v1,v2,v3);
    new_mesh->add_face(v2,v4,v3);
    // Try adding duplicate face here
    new_mesh->add_face(v1,v2,v3);
    return 0;
}
