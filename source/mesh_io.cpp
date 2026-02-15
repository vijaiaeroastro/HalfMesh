#include "triMesh.hpp"
#include "io_format.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

namespace halfMesh {
    void triMesh::save(const std::string &fn) const {
        switch (detect_format_from_path(fn)) {
            case MeshFormat::Gmsh:
                write_gmsh(fn);
                break;
            case MeshFormat::Stl:
                write_stl_ascii(fn);
                break;
            case MeshFormat::Obj:
                write_obj(fn);
                break;
            case MeshFormat::Binary:
                write_binary(fn);
                break;
            case MeshFormat::Vtk:
                write_vtk(fn);
                break;
            default:
                std::cerr << "Unknown format: " << fn << "\n";
                break;
        }
    }

    void triMesh::read(const std::string &filename) {
        switch (detect_format_from_path(filename)) {
            case MeshFormat::Gmsh:
                read_gmsh(filename);
                break;
            case MeshFormat::Stl:
                read_stl(filename);
                break;
            case MeshFormat::Obj:
                read_obj(filename);
                break;
            case MeshFormat::Binary:
                read_binary(filename);
                break;
            case MeshFormat::Vtk:
                std::cerr << "VTK read is not implemented: " << filename << std::endl;
                break;
            default:
                std::cerr << "Unknown format: " << filename << std::endl;
        }
    }

    // — Gmsh —
    void triMesh::read_gmsh(const std::string &fn) {
        clear_data();
        std::ifstream in(fn);
        std::string line;
        bool in_nodes = false, in_elems = false;
        std::unordered_map<unsigned, vertexPtr> tmp;
        while (std::getline(in, line)) {
            if (line == "$Nodes") {
                in_nodes = true;
                continue;
            }
            if (line == "$EndNodes") {
                in_nodes = false;
                continue;
            }
            if (line == "$Elements") {
                in_elems = true;
                continue;
            }
            if (line == "$EndElements") {
                in_elems = false;
                continue;
            }
            if (in_nodes) {
                unsigned idx;
                double x, y, z;
                std::istringstream iss(line);
                if (iss >> idx >> x >> y >> z)
                    tmp[idx] = add_vertex(x, y, z);
            }
            if (in_elems) {
                std::istringstream iss(line);
                unsigned idx, type;
                if (!(iss >> idx >> type)) continue;
                if (type != 2) continue;
                unsigned nt;
                iss >> nt;
                for (unsigned k = 0; k < nt; ++k) iss >> idx;
                unsigned n1, n2, n3;
                iss >> n1 >> n2 >> n3;
                add_face(tmp[n1], tmp[n2], tmp[n3]);
            }
        }
        complete_mesh();
    }

    void triMesh::read_obj(const std::string &fn) {
        clear_data();
        std::ifstream in(fn);
        if (!in) {
            std::cerr << "Could not open OBJ: " << fn << std::endl;
            return;
        }

        std::vector<vertexPtr> obj_vertices;
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string tag;
            if (!(iss >> tag)) {
                continue;
            }

            if (tag == "v") {
                double x = 0.0, y = 0.0, z = 0.0;
                if (iss >> x >> y >> z) {
                    obj_vertices.push_back(add_vertex(x, y, z));
                }
                continue;
            }

            if (tag == "f") {
                std::vector<int> face_indices;
                std::string token;
                while (iss >> token) {
                    if (token.empty()) {
                        continue;
                    }

                    // Accept "i", "i/j", "i//k", "i/j/k" and only use vertex index.
                    const auto slash = token.find('/');
                    const auto index_token = token.substr(0, slash);
                    if (index_token.empty()) {
                        continue;
                    }

                    int idx = std::stoi(index_token);
                    if (idx == 0) {
                        continue;
                    }
                    if (idx < 0) {
                        idx = static_cast<int>(obj_vertices.size()) + idx + 1;
                    }
                    if (idx <= 0 || static_cast<size_t>(idx) > obj_vertices.size()) {
                        continue;
                    }
                    face_indices.push_back(idx - 1); // OBJ is 1-based
                }

                if (face_indices.size() < 3) {
                    continue;
                }

                // Triangulate polygon face as fan (v0, vi, vi+1).
                for (size_t i = 1; i + 1 < face_indices.size(); ++i) {
                    const auto v0 = obj_vertices[face_indices[0]];
                    const auto v1 = obj_vertices[face_indices[i]];
                    const auto v2 = obj_vertices[face_indices[i + 1]];
                    add_face(v0, v1, v2);
                }
            }
        }

        complete_mesh();
    }

    void triMesh::read_binary(const std::string &fn) {
        clear_data();
        std::ifstream in(fn, std::ios::binary);
        std::vector<uint8_t> buf((std::istreambuf_iterator<char>(in)),
                                 std::istreambuf_iterator<char>());
        auto js = nlohmann::json::from_bson(buf);
        for (auto &vv: js["VERTICES"])
            add_vertex(vv[0], vv[1], vv[2]);
        for (auto &ff: js["FACES"])
            add_face(
                get_vertex(ff[0].get<unsigned>()),
                get_vertex(ff[1].get<unsigned>()),
                get_vertex(ff[2].get<unsigned>()));
        vertex_data_store = js["VERTEX_PROPERTIES"];
        edge_data_store = js["EDGE_PROPERTIES"];
        face_data_store = js["FACE_PROPERTIES"];
        complete_mesh();
    }

    // — Writers —
    void triMesh::write_gmsh(const std::string &fn) const {
        std::ofstream out(fn);
        out << "$MeshFormat\n2.2 0 " << sizeof(double) << "\n$EndMeshFormat\n";
        out << "$Nodes\n" << vertices_.size() << "\n";
        for (auto &v: vertices_)
            out << v->get_handle() + 1 << " " << v->get_x() << " " << v->get_y() << " "
                    << v->get_z() << "\n";
        out << "$EndNodes\n$Elements\n" << faces_.size() << "\n";
        for (auto &f: faces_) {
            auto [a,b,c] = f->get_vertices();
            out << f->get_handle() + 1 << " 2 2 0 1 "
                    << a->get_handle() + 1 << " " << b->get_handle() + 1 << " " << c->get_handle() + 1 << "\n";
        }
        out << "$EndElements\n";
    }

    void triMesh::write_obj(const std::string &fn) const {
        std::ofstream out(fn);
        for (auto &v: vertices_)
            out << "v " << v->get_x() << " " << v->get_y() << " " << v->get_z() << "\n";
        for (auto &f: faces_) {
            auto [a,b,c] = f->get_vertices();
            out << "f " << a->get_handle() + 1 << " " << b->get_handle() + 1 << " " << c->get_handle() + 1 << "\n";
        }
    }

    void triMesh::write_binary(const std::string &fn) const {
        nlohmann::json js;
        for (auto &v: vertices_)
            js["VERTICES"].push_back({v->get_x(), v->get_y(), v->get_z()});
        for (auto &f: faces_) {
            auto [a,b,c] = f->get_vertices();
            js["FACES"].push_back({a->get_handle(), b->get_handle(), c->get_handle()});
        }
        js["VERTEX_PROPERTIES"] = vertex_data_store;
        js["EDGE_PROPERTIES"] = edge_data_store;
        js["FACE_PROPERTIES"] = face_data_store;

        auto buf = nlohmann::json::to_bson(js);
        std::ofstream out(fn, std::ios::binary);
        out.write(
            reinterpret_cast<const char *>(buf.data()),
            static_cast<std::streamsize>(buf.size())
        );
    }

    void triMesh::write_vtk(const std::string &fn) const {
        std::ofstream out(fn);
        out << "# vtk DataFile Version 2.0\nHalfMesh VTK\nASCII\nDATASET POLYDATA\n";
        out << "POINTS " << vertices_.size() << " float\n";
        for (auto &v: vertices_)
            out << v->get_x() << " " << v->get_y() << " " << v->get_z() << "\n";
        out << "POLYGONS " << faces_.size() << " " << faces_.size() * 4 << "\n";
        for (auto &f: faces_) {
            auto [a,b,c] = f->get_vertices();
            out << "3 " << a->get_handle() << " " << b->get_handle() << " " << c->get_handle() << "\n";
        }
    }

    //
    // Write ASCII STL
    //
    void triMesh::write_stl_ascii(const std::string &fn) const {
        std::ofstream out(fn);
        if (!out) {
            std::cerr << "Can't open " << fn << std::endl;
            return;
        }
        out << "solid halfMesh" << std::endl;
        for (auto &f: faces_) {
            auto [a,b,c] = f->get_vertices();
            out << "  facet normal " << 0.0 << " " << 0.0 << " " << 0.0 << std::endl;
            out << "    outer loop" << std::endl;
            out << "      vertex " << a->get_x() << " " << a->get_y() << " " << a->get_z() << std::endl;
            out << "      vertex " << b->get_x() << " " << b->get_y() << " " << b->get_z() << std::endl;
            out << "      vertex " << c->get_x() << " " << c->get_y() << " " << c->get_z() << std::endl;
            out << "    endloop" << std::endl;
            out << "  endfacet" << std::endl;
        }
        out << "endsolid halfMesh" << std::endl;
        out.close();
    }

    //
    // Read STL (auto–detect ASCII vs binary)
    //
    void triMesh::read_stl(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Could not open STL file.");
        }

        // Lambda for the hash function
        auto arrayHash = [](const std::array<double, 3> &arr) {
            std::hash<double> hasher;
            return hasher(arr[0]) ^ hasher(arr[1]) ^ hasher(arr[2]);
        };

        // Lambda to read binary STL files
        auto readBinarySTL = [&file, &arrayHash]() {
            char header[80];
            file.read(header, 80);

            uint32_t numTriangles;
            file.read(reinterpret_cast<char *>(&numTriangles), sizeof(numTriangles));

            std::vector<std::array<double, 3> > vertices;
            std::vector<std::array<unsigned int, 3> > triangles;
            std::unordered_map<std::array<double, 3>, unsigned int, decltype(arrayHash)> vertexMap(0, arrayHash);

            for (uint32_t i = 0; i < numTriangles; ++i) {
                float normal[3];
                file.read(reinterpret_cast<char *>(normal), 3 * sizeof(float));

                std::array<unsigned int, 3> triangle;
                for (int j = 0; j < 3; ++j) {
                    float vertex[3];
                    file.read(reinterpret_cast<char *>(vertex), 3 * sizeof(float));
                    std::array<double, 3> vertexArray = {vertex[0], vertex[1], vertex[2]};

                    auto it = vertexMap.find(vertexArray);
                    if (it == vertexMap.end()) {
                        unsigned int index = vertices.size();
                        vertices.push_back(vertexArray);
                        vertexMap[vertexArray] = index;
                        triangle[j] = index;
                    } else {
                        triangle[j] = it->second;
                    }
                }
                triangles.push_back(triangle);

                uint16_t attributeByteCount;
                file.read(reinterpret_cast<char *>(&attributeByteCount), sizeof(attributeByteCount));
            }

            return std::make_pair(vertices, triangles);
        };

        // Lambda to read ASCII STL files
        auto readAsciiSTL = [&file, &arrayHash]() {
            std::vector<std::array<double, 3> > vertices;
            std::vector<std::array<unsigned int, 3> > triangles;
            std::unordered_map<std::array<double, 3>, unsigned int, decltype(arrayHash)> vertexMap(0, arrayHash);

            std::string line;
            std::array<unsigned int, 3> currentTriangle;
            int vertexIndex = 0;

            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string keyword;
                iss >> keyword;

                if (keyword == "vertex") {
                    std::array<double, 3> vertex;
                    iss >> vertex[0] >> vertex[1] >> vertex[2];

                    auto it = vertexMap.find(vertex);
                    if (it == vertexMap.end()) {
                        unsigned int index = vertices.size();
                        vertices.push_back(vertex);
                        vertexMap[vertex] = index;
                        currentTriangle[vertexIndex] = index;
                    } else {
                        currentTriangle[vertexIndex] = it->second;
                    }

                    vertexIndex = (vertexIndex + 1) % 3;
                    if (vertexIndex == 0) {
                        triangles.push_back(currentTriangle);
                    }
                }
            }

            return std::make_pair(vertices, triangles);
        };

        // Check if the file is binary or ASCII
        char header[80];
        file.read(header, 80);
        file.seekg(0, std::ios::beg);

        std::pair<std::vector<std::array<double, 3> >, std::vector<std::array<unsigned int, 3> > > rawResult;
        if (header[0] == 's' && header[1] == 'o' && header[2] == 'l' && header[3] == 'i' && header[4] == 'd') {
            // ASCII STL
            rawResult = readAsciiSTL();
        } else {
            // Binary STL
            rawResult = readBinarySTL();
        }

        // Convert the result to sLib3MFPosition and sLib3MFTriangle
        std::vector<std::array<double,3>> positions;
        for (const auto &vertex: rawResult.first) {
            positions.push_back({static_cast<float>(vertex[0]), static_cast<float>(vertex[1]),
                                               static_cast<float>(vertex[2])});
        }
        std::vector<std::array<int, 3>> triangles;
        for (const auto &triangle: rawResult.second) {
            triangles.push_back({
                static_cast<int>(triangle[0]),
                static_cast<int>(triangle[1]),
                static_cast<int>(triangle[2])});
        }

        std::unordered_map<int, vertexPtr> vertexMap;
        for (unsigned int i = 0; i < positions.size(); ++i) {
            auto iter = positions[i];
            auto currentVertex = add_vertex(iter[0], iter[1], iter[2]);
            vertexMap[i] = currentVertex;
        }
        for (unsigned int i = 0; i < triangles.size(); ++i) {
            auto iter = triangles[i];
            auto currentTriangle = add_face(
                vertexMap[iter[0]],
                vertexMap[iter[1]],
                vertexMap[iter[2]]);
        }

        complete_mesh();
    }

    //
    // Read ASCII STL
    //
    void triMesh::read_stl_ascii(const std::string &fn) {
        clear_data();
        std::ifstream in(fn);
        if (!in) {
            std::cerr << "Can't open " << fn << "\n";
            return;
        }

        // temporary storage of unique vertices
        std::map<std::tuple<double, double, double>, vertexPtr> vmap;
        std::string line;
        std::vector<vertexPtr> tri;

        auto parse_vertex_line = [&](const std::string &l) {
            std::istringstream iss(l);
            std::string tmp;
            double x, y, z;
            if (!(iss >> tmp >> x >> y >> z)) return vertexPtr();
            auto key = std::make_tuple(x, y, z);
            auto it = vmap.find(key);
            if (it != vmap.end()) return it->second;
            auto v = add_vertex(x, y, z);
            vmap[key] = v;
            return v;
        };

        while (std::getline(in, line)) {
            // trim leading spaces
            auto p = line.find_first_not_of(" \t");
            if (p != std::string::npos) line = line.substr(p);

            if (line.rfind("vertex ", 0) == 0) {
                if (auto v = parse_vertex_line(line)) tri.push_back(v);
            } else if (line.rfind("endfacet", 0) == 0) {
                if (tri.size() == 3) add_face(tri[0], tri[1], tri[2]);
                tri.clear();
            }
        }
    }

    //
    // Read binary STL
    //
    void triMesh::read_stl_binary(const std::string &fn) {
        clear_data();
        std::ifstream in(fn, std::ios::binary);
        if (!in) {
            std::cerr << "Can't open " << fn << "\n";
            return;
        }
        // header
        char header[80];
        in.read(header, 80);
        uint32_t triCount;
        in.read(reinterpret_cast<char *>(&triCount), 4);

        // map to dedupe
        std::map<std::tuple<double, double, double>, vertexPtr> vmap;

        auto get_vertex = [&](double x, double y, double z) {
            auto key = std::make_tuple(x, y, z);
            auto it = vmap.find(key);
            if (it != vmap.end()) return it->second;
            auto v = add_vertex(x, y, z);
            vmap[key] = v;
            return v;
        };

        for (uint32_t i = 0; i < triCount; ++i) {
            double nx, ny, nz;
            in.read(reinterpret_cast<char *>(&nx), 4);
            in.read(reinterpret_cast<char *>(&ny), 4);
            in.read(reinterpret_cast<char *>(&nz), 4);
            std::array<vertexPtr, 3> tri;
            for (int k = 0; k < 3; ++k) {
                double x, y, z;
                in.read(reinterpret_cast<char *>(&x), 4);
                in.read(reinterpret_cast<char *>(&y), 4);
                in.read(reinterpret_cast<char *>(&z), 4);
                tri[k] = get_vertex(x, y, z);
                std::cout << x << "->" << y << "->" << z << std::endl;
            }
            // skip attribute bytes
            in.seekg(2, std::ios::cur);
            add_face(tri[0], tri[1], tri[2]);
        }
    }
} // namespace HalfMesh
