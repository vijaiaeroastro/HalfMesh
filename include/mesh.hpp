#pragma once

#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>
#include <json.hpp>
#include <fstream>

namespace HalfMesh {
    class Mesh {
    public:
        Mesh() { clear(); };

        ~Mesh() { clear(); };
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
                new_vertex->set_handle(new_vertex_handle);
                all_vertices.push_back(new_vertex);
                vertex_to_vertex_handle_map[new_vertex] = new_vertex_handle;
                return_handle = new_vertex_handle;
                vertex_handle_to_vertex_map[new_vertex_handle] = new_vertex;
#ifndef NDEBUG
                std::cout << "---> New Vertex added with handle : " << return_handle << std::endl;
#endif
            } else {
                return_handle = vertex_to_vertex_handle_map[new_vertex];
#ifndef NDEBUG
                std::cout << "---> A Vertex exists with handle : " << return_handle << std::endl;
#endif
            }
            return vertex_handle_to_vertex_map[return_handle];
        }

        HalfEdge *add_half_edge(Vertex *v1, Vertex *v2, Face *f1) {
            unsigned int half_edge_handle;
            if (vertex_to_half_edge_map.find(std::make_tuple(v1->handle(), v2->handle())) ==
                vertex_to_half_edge_map.end()) {
                if (all_half_edges.size() == 0) {
                    half_edge_handle = 0;
                } else {
                    half_edge_handle = all_half_edges.size();
                }
                HalfEdge *new_half_edge = new HalfEdge(v1, v2);
                new_half_edge->set_handle(half_edge_handle);
                new_half_edge->set_parent_face(f1->handle());
                if (vertex_to_half_edge_map.find(std::make_tuple(v2->handle(), v1->handle())) !=
                    vertex_to_half_edge_map.end()) {
                    unsigned int existing_half_edge = vertex_to_half_edge_map[std::make_tuple(v2->handle(),
                                                                                              v1->handle())]->handle();
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
            } else {
                half_edge_handle = vertex_to_half_edge_map[std::make_tuple(v1->handle(), v2->handle())]->handle();
#ifndef NDEBUG
                std::cout << "--->--->---> A Half Edge exists with handle : " << half_edge_handle << std::endl;
#endif
            }
            return half_edge_handle_to_half_edge_map[half_edge_handle];
        }

        Edge *add_edge(Vertex *v1, Vertex *v2, Face *f1) {
            unsigned int edge_handle;
            if (vertices_to_edge_handle_map.find(std::make_tuple(v1->handle(), v2->handle())) ==
                vertices_to_edge_handle_map.end()) {
                if (vertices_to_edge_handle_map.find(std::make_tuple(v2->handle(), v1->handle())) ==
                    vertices_to_edge_handle_map.end()) {
                    if (all_edges.size() == 0) {
                        edge_handle = 0;
                    } else {
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
                } else {
                    edge_handle = vertices_to_edge_handle_map[std::make_tuple(v2->handle(), v1->handle())];
#ifndef NDEBUG
                    std::cout << "--->---> An Edge exists with handle : " << edge_handle << std::endl;
#endif
                    Edge *existing_edge = edge_handle_to_edge_map[edge_handle];
                    HalfEdge *he = add_half_edge(v1, v2, f1);
                    he->set_parent_edge(edge_handle);
                    edge_to_one_half_edge_map[existing_edge] = he;
                }
            } else {
                edge_handle = vertices_to_edge_handle_map[std::make_tuple(v1->handle(), v2->handle())];
#ifndef NDEBUG
                std::cout << "--->---> An Edge exists with handle : " << edge_handle << std::endl;
#endif
            }
            return edge_handle_to_edge_map[edge_handle];
        }

        Face *add_face(Vertex *v1, Vertex *v2, Vertex *v3) {
            unsigned int face_handle;
            if (vertices_to_face_handle_map.find(std::make_tuple(v1->handle(), v2->handle(), v3->handle())) ==
                vertices_to_face_handle_map.end()) {
                if (all_faces.size() == 0) {
                    face_handle = 0;
                } else {
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
            } else {
                face_handle = vertices_to_face_handle_map[std::make_tuple(v1->handle(), v2->handle(), v3->handle())];
#ifndef NDEBUG
                std::cout << "---> A Face exists with handle : " << face_handle << std::endl;
#endif
            }
            return face_handle_to_face_map[face_handle];
        }


    private:
        void establish_connectivity() {
            for (auto iter = face_to_one_half_edge_map.begin(); iter != face_to_one_half_edge_map.end(); ++iter) {
                Face *face = iter->first;
                HalfEdge *halfEdge = iter->second;
                face->set_one_half_edge(halfEdge);
            }
            for (auto iter = edge_to_one_half_edge_map.begin(); iter != edge_to_one_half_edge_map.end(); ++iter) {
                Edge *edge = iter->first;
                HalfEdge *halfEdge = iter->second;
                edge->set_one_half_edge(halfEdge);
            }
            for (unsigned int i = 0; i < all_half_edges.size(); ++i) {
                HalfEdge *half_edge = all_half_edges.at(i);
                if (half_edge->get_opposing_half_edge() == std::numeric_limits<unsigned int>::max()) {
                    half_edge->set_boundary(true);
                } else {
                    half_edge->set_boundary(false);
                }
            }
            for (unsigned int i = 0; i < all_edges.size(); ++i) {
                Edge *edge = all_edges.at(i);
                HalfEdge *halfEdge = edge->get_one_half_edge();
                if (halfEdge->is_boundary()) {
                    edge->set_boundary(true);
                } else {
                    edge->set_boundary(false);
                }

            }
        }

    public:
        std::vector<std::vector<unsigned int> > get_edge_loops(std::string property_name) {
            std::vector<Edge *> property_search_space;
            for (unsigned int i = 0; i < all_edges.size(); ++i) {
                if (edge_data_store[property_name][i]) {
                    property_search_space.push_back(all_edges.at(i));
                }
            }
            for (unsigned int i = 0; i < property_search_space.size(); ++i) {

            }
        }

    public:
        void complete_mesh() {
            if (all_faces.size() > 0) {
                establish_connectivity();
            }
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

    private:
        void clear() {
            all_faces.clear();
            all_edges.clear();
            all_half_edges.clear();
            all_vertices.clear();
            vertex_to_vertex_handle_map.clear();
            vertices_to_face_handle_map.clear();
            vertices_to_edge_handle_map.clear();
            vertex_handle_to_vertex_map.clear();
            edge_handle_to_edge_map.clear();
            half_edge_handle_to_half_edge_map.clear();
            face_handle_to_face_map.clear();
            face_to_one_half_edge_map.clear();
            edge_to_one_half_edge_map.clear();
            vertex_to_half_edge_map.clear();
            vertex_data_store.clear();
            edge_data_store.clear();
            half_edge_data_store.clear();
            face_data_store.clear();
        }


    public:
        Face *get_face(unsigned int i) {
            Face *new_face = NULL_FACE;
            if (face_handle_to_face_map.find(i) != face_handle_to_face_map.end()) {
                return face_handle_to_face_map[i];
            } else {
                return new_face;
            }
        }

        Edge *get_edge(unsigned int i) {
            Edge *new_edge = NULL_EDGE;
            if (edge_handle_to_edge_map.find(i) != edge_handle_to_edge_map.end()) {
                return edge_handle_to_edge_map[i];
            } else {
                return new_edge;
            }
        }

        Vertex *get_vertex(unsigned int i) {
            Vertex *new_vertex = NULL_VERTEX;
            if (vertex_handle_to_vertex_map.find(i) != vertex_handle_to_vertex_map.end()) {
                return vertex_handle_to_vertex_map[i];
            } else {
                return new_vertex;
            }
        }

        HalfEdge *get_half_edge(unsigned int i) {
            HalfEdge *new_half_edge = NULL_HALF_EDGE;
            if (half_edge_handle_to_half_edge_map.find(i) != half_edge_handle_to_half_edge_map.end()) {
                return half_edge_handle_to_half_edge_map[i];
            } else {
                return new_half_edge;
            }
        }

    public:
        HalfEdge *get_next_half_edge(HalfEdge *input_half_edge, Face *input_face) {
            HalfEdge *return_edge = NULL_HALF_EDGE;
            std::unordered_set<unsigned int> outgoing_half_edges = input_half_edge->get_vertex_two()->get_outgoing_half_edges();
            for (auto iter = outgoing_half_edges.begin(); iter != outgoing_half_edges.end(); ++iter) {
                HalfEdge *current_outgoing_half_edge = half_edge_handle_to_half_edge_map[*iter];
                if (current_outgoing_half_edge == input_half_edge) {
                    continue;
                }
                if (current_outgoing_half_edge->get_parent_face() == input_face->handle()) {
                    return_edge = current_outgoing_half_edge;
                }
            }
            return return_edge;
        }

        HalfEdge *get_previous_half_edge(HalfEdge *input_half_edge, Face *input_face) {
            HalfEdge *return_edge = NULL_HALF_EDGE;
            std::unordered_set<unsigned int> incoming_half_edges = input_half_edge->get_vertex_one()->get_incoming_half_edges();
            for (auto iter = incoming_half_edges.begin(); iter != incoming_half_edges.end(); ++iter) {
                HalfEdge *current_incoming_half_edge = half_edge_handle_to_half_edge_map[*iter];
                if (current_incoming_half_edge == input_half_edge) {
                    continue;
                }
                if (current_incoming_half_edge->get_parent_face() == input_face->handle()) {
                    return_edge = current_incoming_half_edge;
                }
            }
            return return_edge;
        }

    public:
        template<typename T>
        PROPERTY_STATUS add_vertex_property(std::string property_name, T initialization_value) {
            if (vertex_data_store.contains(property_name)) {
                return PROPERTY_STATUS::PROPERTY_EXISTS;
            } else {
                for (unsigned int i = 0; i < all_vertices.size(); ++i) {
                    vertex_data_store[property_name][all_vertices.at(i)->handle()] = initialization_value;
                }
            }
        }

        template<typename T>
        PROPERTY_STATUS add_edge_property(std::string property_name, T initialization_value) {
            if (edge_data_store.contains(property_name)) {
                return PROPERTY_STATUS::PROPERTY_EXISTS;
            } else {
                for (unsigned int i = 0; i < all_edges.size(); ++i) {
                    edge_data_store[property_name][all_edges.at(i)->handle()] = initialization_value;
                }
                return PROPERTY_STATUS::PROPERTY_ADDED;
            }
        }

        template<typename T>
        PROPERTY_STATUS add_half_edge_property(std::string property_name, T initialization_value) {
            if (half_edge_data_store.contains(property_name)) {
                return PROPERTY_STATUS::PROPERTY_EXISTS;
            } else {
                for (unsigned int i = 0; i < all_half_edges.size(); ++i) {
                    half_edge_data_store[property_name][all_half_edges.at(i)->handle()] = initialization_value;
                }
            }
        }

        template<typename T>
        PROPERTY_STATUS add_face_property(std::string property_name, T initialization_value) {
            if (face_data_store.contains(property_name)) {
                return PROPERTY_STATUS::PROPERTY_EXISTS;
            } else {
                for (unsigned int i = 0; i < all_faces.size(); ++i) {
                    face_data_store[property_name][all_faces.at(i)->handle()] = initialization_value;
                }
            }
        }

        PROPERTY_STATUS delete_property(std::string property_name, MESH_ENTITY_TYPE mesh_entity) {
            if (mesh_entity == MESH_ENTITY_TYPE::M_VERTEX) {
                if (vertex_data_store.contains(property_name)) {
                    vertex_data_store.erase(property_name);
                }
                if (!vertex_data_store.contains(property_name)) {
                    return PROPERTY_STATUS::PROPERTY_DELETED;
                } else {
                    return PROPERTY_STATUS::PROPERTY_COULD_NOT_BE_DELETED;
                }
            } else if (mesh_entity == MESH_ENTITY_TYPE::M_EDGE) {
                if (edge_data_store.contains(property_name)) {
                    edge_data_store.erase(property_name);
                }
                if (!edge_data_store.contains(property_name)) {
                    return PROPERTY_STATUS::PROPERTY_DELETED;
                } else {
                    return PROPERTY_STATUS::PROPERTY_COULD_NOT_BE_DELETED;
                }
            } else if (mesh_entity == MESH_ENTITY_TYPE::M_HALF_EDGE) {
                if (half_edge_data_store.contains(property_name)) {
                    half_edge_data_store.erase(property_name);
                }
                if (!half_edge_data_store.contains(property_name)) {
                    return PROPERTY_STATUS::PROPERTY_DELETED;
                } else {
                    return PROPERTY_STATUS::PROPERTY_COULD_NOT_BE_DELETED;
                }
            } else if (mesh_entity == MESH_ENTITY_TYPE::M_FACE) {
                if (face_data_store.contains(property_name)) {
                    face_data_store.erase(property_name);
                }
                if (!face_data_store.contains(property_name)) {
                    return PROPERTY_STATUS::PROPERTY_DELETED;
                } else {
                    return PROPERTY_STATUS::PROPERTY_COULD_NOT_BE_DELETED;
                }
            } else {
                return PROPERTY_STATUS::PROPERTY_DOES_NOT_EXIST;
            }
        }

    public:
        template<typename T>
        void set_vertex_property(std::string property_name, unsigned int handle, T value) {
            vertex_data_store[property_name][handle] = value;
        }

        template<typename T>
        void set_edge_property(std::string property_name, unsigned int handle, T value) {
            edge_data_store[property_name][handle] = value;
        }

        template<typename T>
        void set_half_edge_property(std::string property_name, unsigned int handle, T value) {
            half_edge_data_store[property_name][handle] = value;
        }

        template<typename T>
        void set_face_property(std::string property_name, unsigned int handle, T value) {
            face_data_store[property_name][handle] = value;
        }

    public:
        template<typename T>
        T get_vertex_property(std::string property_name, unsigned int handle) {
            return vertex_data_store[property_name][handle];
        }

        template<typename T>
        T get_edge_property(std::string property_name, unsigned int handle) {
            return edge_data_store[property_name][handle];
        }


        template<typename T>
        T get_half_edge_property(std::string property_name, unsigned int handle) {
            return half_edge_data_store[property_name][handle];
        }


        template<typename T>
        T get_face_property(std::string property_name, unsigned int handle) {
            return face_data_store[property_name][handle];
        }

    public:
        void save(const std::string mesh_name) {
            MESH_TYPE current_mesh_type = guess_mesh_format(mesh_name);
            if (current_mesh_type == MESH_TYPE::BINARY_MESH) {
                write_binary_mesh(mesh_name);
            } else if (current_mesh_type == MESH_TYPE::GMSH) {
                write_gmsh(mesh_name);
            } else if (current_mesh_type == MESH_TYPE::OBJ) {
                write_obj(mesh_name);
            } else {
                std::cout << "Unknown data type. Use GMSH / OBJ / BM mesh formats" << std::endl;
            }
        }

        void read(const std::string mesh_name) {
            MESH_TYPE current_mesh_type = guess_mesh_format(mesh_name);
            if (current_mesh_type == MESH_TYPE::BINARY_MESH) {
                read_binary_mesh(mesh_name);
            } else if (current_mesh_type == MESH_TYPE::GMSH) {
                read_gmsh(mesh_name);
            } else if (current_mesh_type == MESH_TYPE::OBJ) {
                read_obj(mesh_name);
            } else {
                std::cout << "Unknown data type. Use GMSH / OBJ / BM mesh formats" << std::endl;
            }
        }

    private:
        void read_gmsh(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> GMSH READER " << mesh_name << std::endl;
#endif
            clear();
            std::ifstream gmsh_file(mesh_name);
            std::string current_line;
            bool begin_vertex_data = false;
            bool begin_face_data = false;
            std::unordered_map<unsigned int, Vertex *> vertex_handle_to_vertex_map;
            while (std::getline(gmsh_file, current_line)) {
                if (is_substring(current_line, "$Nodes")) {
                    begin_vertex_data = true;
                }
                if (is_substring(current_line, "$EndNodes")) {
                    begin_vertex_data = false;
                }
                if (is_substring(current_line, "$Elements")) {
                    begin_face_data = true;
                }
                if (is_substring(current_line, "$EndElements")) {
                    begin_face_data = false;
                }
                if (begin_vertex_data) {
                    std::istringstream iss(current_line);
                    int vertex_counter;
                    double x, y, z;
                    if ((iss >> vertex_counter >> x >> y >> z)) {
                        Vertex *vertex = new Vertex(x, y, z);
                        add_vertex(vertex);
                        vertex_handle_to_vertex_map[vertex->handle() + 1] = vertex;
                    }
                }
                if (begin_face_data) {
                    std::vector<std::string> split_lines = split_string(current_line, " ", true);
                    if (split_lines.size() > 1) {
                        if (split_lines.at(1) == "2") {
                            unsigned int v1 = atoi(split_lines.at(split_lines.size() - 3).c_str());
                            unsigned int v2 = atoi(split_lines.at(split_lines.size() - 2).c_str());
                            unsigned int v3 = atoi(split_lines.at(split_lines.size() - 1).c_str());
                            add_face(vertex_handle_to_vertex_map[v1], vertex_handle_to_vertex_map[v2],
                                     vertex_handle_to_vertex_map[v3]);
                        }

                    }
                }
            }
            complete_mesh();
            gmsh_file.close();
        }

        void read_binary_mesh(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> BINARY MESH READER " << mesh_name << std::endl;
#endif
            clear();
            using json = nlohmann::json;
            std::ifstream stream(mesh_name, std::ios::in | std::ios::binary);
            std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            json binary_json = json::from_bson(contents);
            json json_vertices = binary_json["VERTICES"];
            for (json::iterator it = json_vertices.begin(); it != json_vertices.end(); ++it) {
                Vertex *vertex = new Vertex((*it)[0], (*it)[1], (*it)[2]);
                add_vertex(vertex);
            }
            nlohmann::json json_faces = binary_json["FACES"];
            for (json::iterator it = json_faces.begin(); it != json_faces.end(); ++it) {
                unsigned int f1, f2, f3;
                f1 = (*it)[0];
                f2 = (*it)[1];
                f3 = (*it)[2];
                add_face(get_vertex(f1), get_vertex(f2), get_vertex(f3));
            }
            vertex_data_store = binary_json["VERTEX_PROPERTIES"];
            edge_data_store = binary_json["EDGE_PROPERTIES"];
            half_edge_data_store = binary_json["HALF_EDGE_PROPERTIES"];
            face_data_store = binary_json["FACE_PROPERTIES"];
            complete_mesh();
            stream.close();
        }

        void read_obj(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> OBJ READER " << mesh_name << std::endl;
#endif
            clear();
            std::ifstream obj_file(mesh_name);
            std::string current_line;
            unsigned int unknown_vertices = 0;
            unsigned int unknown_faces = 0;
            while (std::getline(obj_file, current_line)) {
                if (strutil::starts_with(current_line, "v ")) {
                    std::vector<std::string> vertex_string = split_string(current_line, " ", true);
                    if (vertex_string.size() == 4) {
                        double x, y, z;
                        x = std::stod(vertex_string.at(1));
                        y = std::stod(vertex_string.at(2));
                        z = std::stod(vertex_string.at(3));
                        Vertex *vertex = new Vertex(x, y, z);
                        add_vertex(vertex);
                    } else {
                        ++unknown_vertices;
                    }
                }
                if (strutil::starts_with(current_line, "f ")) {
                    std::vector<std::string> face_string = split_string(current_line, " ", true);
                    if (face_string.size() == 4) {
                        unsigned int e1, e2, e3;
                        if (strutil::contains(face_string.at(1), "//")) {
                            std::vector<std::string> sub_split_e1 = split_string(face_string.at(1), "//", true);
                            e1 = static_cast< unsigned int >(std::stoi(sub_split_e1.at(0)));
                        } else {
                            e1 = static_cast< unsigned int >(std::stoi(face_string.at(1)));
                        }
                        if (strutil::contains(face_string.at(2), "//")) {
                            std::vector<std::string> sub_split_e2 = split_string(face_string.at(2), "//", true);
                            e2 = static_cast< unsigned int >(std::stoi(sub_split_e2.at(0)));
                        } else {
                            e2 = static_cast< unsigned int >(std::stoi(face_string.at(2)));
                        }
                        if (strutil::contains(face_string.at(3), "//")) {
                            std::vector<std::string> sub_split_e3 = split_string(face_string.at(3), "//", true);
                            e3 = static_cast< unsigned int >(std::stoi(sub_split_e3.at(0)));
                        } else {
                            e3 = static_cast< unsigned int >(std::stoi(face_string.at(3)));
                        }
                        add_face(get_vertex(e1 - 1), get_vertex(e2 - 1), get_vertex(e3 - 1));
                    } else {
                        ++unknown_faces;
                    }
                }
            }
            if ((unknown_vertices == 0) && (unknown_faces == 0)) {
                complete_mesh();
            } else {
                std::cout << "The input geometry has " << unknown_vertices << " invalid vertices (higher than 3d) and "
                          << unknown_faces << " invalid faces (arbitrary polygons)" << std::endl;
            }
            obj_file.close();

        }

    private:
        void write_gmsh(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> GMSH WRITER " << mesh_name << std::endl;
#endif
        }

        void write_binary_mesh(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> BINARY MESH WRITER " << mesh_name << std::endl;
#endif
            nlohmann::json combined_json_file;
            for (unsigned int i = 0; i < all_vertices.size(); ++i) {
                Vertex *current_vertex = all_vertices.at(i);
                combined_json_file["VERTICES"][i] = {current_vertex->get_x(), current_vertex->get_y(),
                                                     current_vertex->get_z()};
            }
            for (unsigned int i = 0; i < all_faces.size(); ++i) {
                Face *current_face = all_faces.at(i);
                combined_json_file["FACES"][i] = {current_face->get_vertex_one()->handle(),
                                                  current_face->get_vertex_two()->handle(),
                                                  current_face->get_vertex_three()->handle()};
            }
            combined_json_file["VERTEX_PROPERTIES"] = vertex_data_store;
            combined_json_file["EDGE_PROPERTIES"] = edge_data_store;
            combined_json_file["HALF_EDGE_PROPERTIES"] = half_edge_data_store;
            combined_json_file["FACE_PROPERTIES"] = face_data_store;
            std::remove(mesh_name.c_str());
            std::vector<std::uint8_t> v_bson = nlohmann::json::to_bson(combined_json_file);
            std::ofstream mesh_output(mesh_name, std::ios::out | std::ios::binary);
            mesh_output.write((char *) &v_bson[0], v_bson.size() * sizeof(std::uint8_t));
            mesh_output.close();
        }

        void write_obj(const std::string mesh_name) {
#ifndef NDEBUG
            std::cout << "---> OBJ MESH WRITER " << mesh_name << std::endl;
#endif
            std::remove(mesh_name.c_str());
            std::ofstream obj_file(mesh_name);
            obj_file << "# Half Mesh OBJ Writer\n";
            for (unsigned int i = 0; i < all_vertices.size(); ++i) {
                Vertex *current_vertex = all_vertices.at(i);
                obj_file << "v " << current_vertex->get_x() << " " << current_vertex->get_y() << " "
                         << current_vertex->get_z() << std::endl;
            }
            obj_file << "# " << all_vertices.size() << " vertices" << std::endl;
            for (unsigned int i = 0; i < all_faces.size(); ++i) {
                Face *current_face = all_faces.at(i);
                obj_file << "f " << current_face->get_vertex_one()->handle() << " "
                         << current_face->get_vertex_two()->handle() << " "
                         << current_face->get_vertex_three()->handle() << std::endl;
            }
            obj_file << "# " << all_faces.size() << " faces" << std::endl;
            obj_file.close();
        }


    private:
        std::vector<Face *> all_faces;
        std::vector<Edge *> all_edges;
        std::vector<HalfEdge *> all_half_edges;
        std::vector<Vertex *> all_vertices;
        std::unordered_map<Vertex *, unsigned int> vertex_to_vertex_handle_map;
        std::unordered_map<unsigned int, Vertex *> vertex_handle_to_vertex_map;
        std::unordered_map<unsigned int, Edge *> edge_handle_to_edge_map;
        std::unordered_map<unsigned int, HalfEdge *> half_edge_handle_to_half_edge_map;
        std::unordered_map<unsigned int, Face *> face_handle_to_face_map;
        std::unordered_map<Face *, HalfEdge *> face_to_one_half_edge_map;
        std::unordered_map<Edge *, HalfEdge *> edge_to_one_half_edge_map;

    private:
        typedef std::unordered_map<const twin_type_t, unsigned int, twin_key_hash, twin_key_equal> special_map_twin;
        typedef std::unordered_map<const twin_type_t, HalfEdge *, twin_key_hash, twin_key_equal> twin_map_he_special;
        special_map_three vertices_to_face_handle_map;
        special_map_twin vertices_to_edge_handle_map;
        twin_map_he_special vertex_to_half_edge_map;

    private:
        nlohmann::json vertex_data_store;
        nlohmann::json edge_data_store;
        nlohmann::json half_edge_data_store;
        nlohmann::json face_data_store;

    };
}