#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace halfMesh {
    enum class MeshFormat {
        Auto = 0,
        Gmsh = 100,
        Stl = 200,
        Obj = 250,
        Binary = 300,
        Vtk = 500,
        Unknown = 999
    };

    inline MeshFormat detect_format_from_path(const std::string &filename) {
        auto lower = filename;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](const unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });

        if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".msh") == 0) return MeshFormat::Gmsh;
        if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".stl") == 0) return MeshFormat::Stl;
        if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".obj") == 0) return MeshFormat::Obj;
        if (lower.size() >= 3 && lower.compare(lower.size() - 3, 3, ".bm") == 0) return MeshFormat::Binary;
        if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".vtk") == 0) return MeshFormat::Vtk;
        return MeshFormat::Unknown;
    }

    // Compatibility aliases for older API surface.
    using MeshType = MeshFormat;

    inline MeshFormat guess_mesh_format(const std::string &filename) {
        return detect_format_from_path(filename);
    }
} // namespace halfMesh
