#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>
#include <mesh.hpp>
#include <fstream>
#include <sstream>

namespace HalfMesh
{
    class MeshIO
    {
        public:
            MeshIO(){};
            ~MeshIO(){};

        public:
            void read_mesh(Mesh* input_mesh, const std::string _input_mesh_name, MESH_TYPE mesh_type)
            {
                input_file_name = _input_mesh_name;
                if(mesh_type == MESH_TYPE::GMSH)
                {
                    read_gmsh(input_mesh);
                }
                if(mesh_type == MESH_TYPE::OBJ)
                {
                    read_obj(input_mesh);
                }
            }

            void write_mesh(Mesh* output_mesh, const std::string _output_mesh_name, MESH_TYPE mesh_type)
            {
                output_file_name = _output_mesh_name;
                if(mesh_type == MESH_TYPE::GMSH)
                {
                    write_gmsh(output_mesh);
                }
                if(mesh_type = MESH_TYPE::OBJ)
                {
                    write_obj(output_mesh);
                }
            }

        private:
            void read_gmsh(Mesh* input_mesh)
            {
                std::ifstream gmsh_file(input_file_name);
                std::string current_line;
                bool begin_vertex_data = false;
                bool begin_face_data = false;
                std::unordered_map< unsigned int, Vertex* > vertex_handle_to_vertex_map;
                while (std::getline(gmsh_file, current_line))
                {
                    if(is_substring(current_line, "$Nodes"))
                    {
                        begin_vertex_data = true;
                    }
                    if(is_substring(current_line, "$EndNodes"))
                    {
                        begin_vertex_data = false;
                    }
                    if(is_substring(current_line, "$Elements"))
                    {
                        begin_face_data = true;
                    }
                    if(is_substring(current_line, "$EndElements"))
                    {
                        begin_face_data = false;
                    }
                    if(begin_vertex_data)
                    {
                        std::istringstream iss(current_line);
                        int vertex_counter;
                        double x,y,z;
                        if ((iss >> vertex_counter >> x >> y >> z))
                        {
                            Vertex* vertex = new Vertex(x,y,z);
                            input_mesh->add_vertex(vertex);
                            vertex_handle_to_vertex_map[vertex->handle() + 1] = vertex;
                        }
                    }
                    if(begin_face_data)
                    {
                        std::vector< std::string > split_lines = split_string(current_line, " ", true);
                        if(split_lines.size() > 1)
                        {
                            if(split_lines.at(1) == "2")
                            {
                                unsigned int v1 = atoi(split_lines.at(split_lines.size() - 3).c_str());
                                unsigned int v2 = atoi(split_lines.at(split_lines.size() - 2).c_str());
                                unsigned int v3 = atoi(split_lines.at(split_lines.size() - 1).c_str());
                                input_mesh->add_face(vertex_handle_to_vertex_map[v1], vertex_handle_to_vertex_map[v2], vertex_handle_to_vertex_map[v3]);
                            }

                        }
                    }
                }
                input_mesh->complete_mesh();

            }

            void read_obj(Mesh* input_mesh)
            {

            }

        private:
            void write_gmsh(Mesh* output_mesh)
            {

            }

            void write_obj(Mesh* output_mesh)
            {

            }


        private:
            std::string input_file_name;
            std::string output_file_name;
    };
}