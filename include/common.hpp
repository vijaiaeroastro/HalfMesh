#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace halfMesh {
    // Forward declarations
    class vertex;
    class face;
    class edge;
    class halfedge;
    class triMesh;

    // Set pointers to these forward declarations too
    using vertexPtr = std::shared_ptr<vertex>;
    using halfEdgePtr = std::shared_ptr<halfedge>;
    using edgePtr = std::shared_ptr<edge>;
    using facePtr = std::shared_ptr<face>;
    using Vec3 = std::array<double, 3>;

    struct AABB {
        Vec3 min_corner{
            std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity()
        };
        Vec3 max_corner{
            -std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity()
        };
        bool empty = true;

        void extend(const Vec3 &p) {
            if (empty) {
                min_corner = p;
                max_corner = p;
                empty = false;
                return;
            }
            for (size_t i = 0; i < 3; ++i) {
                min_corner[i] = std::min(min_corner[i], p[i]);
                max_corner[i] = std::max(max_corner[i], p[i]);
            }
        }
    };


    // --- Entity identifiers ---
    enum class EntityType {
        Vertex,
        Edge,
        Face,
        HalfEdge
    };

    // --- Property API return codes ---
    enum class PropertyStatus {
        Added,
        Exists,
        Deleted,
        CouldNotDelete,
        DoesNotExist,
        CouldNotAdd
    };

    // Some string related utilities
    // Convert a copy of s to lowercase
    inline std::string to_lower(std::string s) {
        std::transform(
            s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );
        return s;
    }

    // Convert a copy of s to uppercase
    inline std::string to_upper(std::string s) {
        std::transform(
            s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); }
        );
        return s;
    }

    // Does s start with prefix?
    inline bool starts_with(const std::string &s, const std::string &prefix) {
        return s.size() >= prefix.size()
               && std::equal(prefix.begin(), prefix.end(), s.begin());
    }

    // Does s end with suffix?
    inline bool ends_with(const std::string &s, const std::string &suffix) {
        return s.size() >= suffix.size()
               && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
    }

    // Helpers
    inline bool is_substring(const std::string &str, const std::string &sub) {
        return str.find(sub) != std::string::npos;
    }

    inline std::vector<std::string>
    split_string(const std::string &str, const std::string &delim, bool trim_empty = false) {
        std::vector<std::string> tokens;
        size_t start = 0, pos;
        while ((pos = str.find(delim, start)) != std::string::npos) {
            if (!trim_empty || pos > start)
                tokens.emplace_back(str.substr(start, pos - start));
            start = pos + delim.size();
        }
        if (!trim_empty || start < str.size())
            tokens.emplace_back(str.substr(start));
        return tokens;
    }

    template<typename T>
    constexpr T squared(T v) { return v * v; }
}
