#include <mesh.hpp>

int main()
{
    using namespace HalfMesh;
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
    // Complete the mesh here
    new_mesh->complete_mesh();
    // Loop through faces
    for(unsigned int i = 0; i < new_mesh->get_faces().size(); ++i)
    {
        Face *new_face = new_mesh->get_faces().at(i);
        std::cout << "One half edge for face F: " << new_face->face_handle << " HE : " << new_face->one_half_edge->half_edge_handle << std::endl;
    }
    // Loop through vertices
    for(unsigned int i = 0; i < new_mesh->get_vertices().size(); ++i)
    {
        Vertex* new_vertex = new_mesh->get_vertices().at(i);
        std::cout << "Vertex " << i+1 << " has " << new_vertex->incoming_half_edges.size() << " incoming half edges and " << new_vertex->outgoing_half_edges.size() << " outgoing half edges" << std::endl;
    }
    // Loop through edges and find boundaries
    unsigned int count = 0;
    for(unsigned int i = 0; i < new_mesh->get_edges().size(); ++i)
    {
        Edge* new_edge = new_mesh->get_edges().at(i);
        if(new_edge->boundary_edge)
        {
            count = count + 1;
        }
    }
    std::cout << "There is a total of " << count << " boundary edges in this mesh" << std::endl;
    return 0;
}
