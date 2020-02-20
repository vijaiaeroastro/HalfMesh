#include <mesh.hpp>

int main()
{
    using namespace HalfMesh;
    Mesh *new_mesh = new Mesh;
    Vertex *v1 = new Vertex(0.0,0.0,0.0);
    Vertex *v2 = new Vertex(1.0,0.0,0.0);
    Vertex *v3 = new Vertex(0.0,0.5,0.0);
    Vertex *v4 = new Vertex(1.5,0.5,0.0);
    Vertex *v5 = new Vertex(2.5,0.0,0.0);
    new_mesh->add_vertex(v1);
    new_mesh->add_vertex(v2);
    new_mesh->add_vertex(v3);
    new_mesh->add_vertex(v4);
    new_mesh->add_vertex(v5);
    // add face
    new_mesh->add_face(v1,v2,v3);
    new_mesh->add_face(v2,v4,v3);
    new_mesh->add_face(v2,v5,v4);
    // Complete the mesh here
    new_mesh->complete_mesh();
    // Loop through half edges in a face
    for(unsigned int i = 0; i < new_mesh->get_faces().size(); ++i)
    {
        Face *new_face = new_mesh->get_faces().at(i);
        std::cout << "One half edge for face F: " << new_face->handle() << " HE : " << new_face->get_one_half_edge()->handle() << std::endl;
        unsigned int he_1, he_2, he_3, he_4;
        he_1 = new_face->get_one_half_edge()->handle();
        he_2 = new_mesh->get_next_half_edge(new_mesh->get_half_edge(he_1), new_face)->handle();
        he_3 = new_mesh->get_next_half_edge(new_mesh->get_half_edge(he_2), new_face)->handle();
        he_4 = new_mesh->get_next_half_edge(new_mesh->get_half_edge(he_3), new_face)->handle();
        std::cout << he_1 << "->" << he_2 << "->" << he_3 << "->" << he_4 << std::endl;
    }
    // Loop through vertices
    for(unsigned int i = 0; i < new_mesh->get_vertices().size(); ++i)
    {
        Vertex* new_vertex = new_mesh->get_vertices().at(i);
        std::cout << "Vertex " << i+1 << " has " << new_vertex->get_incoming_half_edges().size() << " incoming half edges and " << new_vertex->get_outgoing_half_edges().size() << " outgoing half edges" << std::endl;
        print_set(new_vertex->get_outgoing_half_edges());
        print_set(new_vertex->get_incoming_half_edges());
    }
    // Loop through edges and find boundaries
    unsigned int count = 0;
    for(unsigned int i = 0; i < new_mesh->get_edges().size(); ++i)
    {
        Edge* new_edge = new_mesh->get_edges().at(i);
        if(new_edge->is_boundary())
        {
            count = count + 1;
        }
    }
    std::cout << "There is a total of " << count << " boundary edges in this mesh" << std::endl;
    // Loop through half edges and find boundaries
    count = 0;
    for(unsigned int i = 0; i < new_mesh->get_half_edges().size(); ++i)
    {
        HalfEdge* halfEdge = new_mesh->get_half_edges().at(i);
        if(halfEdge->is_boundary())
        {
            count = count + 1;
        }
    }
    std::cout << "There is a total of " << count << " boundary half edges in this mesh" << std::endl;
    return 0;
}
