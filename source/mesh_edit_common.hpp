#pragma once

#include "triMesh.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace halfMesh::detail {
    inline EdgeKey make_edge_key(unsigned i, unsigned j) {
        if (i > j) std::swap(i, j);
        return {i, j};
    }

    inline FaceKey make_face_key(unsigned a, unsigned b, unsigned c) {
        std::array<unsigned, 3> v{a, b, c};
        std::sort(v.begin(), v.end());
        return {v[0], v[1], v[2]};
    }

    inline std::array<vertexPtr, 3> get_oriented_face_vertices(const facePtr &f) {
        std::array<vertexPtr, 3> out{nullptr, nullptr, nullptr};
        if (!f) {
            return out;
        }

        const auto start = f->get_one_half_edge();
        if (start) {
            const auto n1 = start->next();
            const auto n2 = n1 ? n1->next() : nullptr;
            if (n1 && n2 && n2->next() == start) {
                out[0] = start->get_vertex_one();
                out[1] = start->get_vertex_two();
                out[2] = n1->get_vertex_two();
                if (out[0] && out[1] && out[2]) {
                    return out;
                }
            }
        }

        auto [a, b, c] = f->get_vertices();
        out[0] = a;
        out[1] = b;
        out[2] = c;
        return out;
    }

    inline bool face_contains_handle(const std::array<vertexPtr, 3> &verts, const unsigned h) {
        for (const auto &v: verts) {
            if (v && v->get_handle() == h) {
                return true;
            }
        }
        return false;
    }

    inline std::array<unsigned, 3> to_handle_triplet(const std::array<vertexPtr, 3> &verts) {
        return {
            verts[0]->get_handle(),
            verts[1]->get_handle(),
            verts[2]->get_handle()
        };
    }

    inline bool try_get_property_value_for_handle(const nlohmann::json &bucket,
                                                  const unsigned handle,
                                                  nlohmann::json &out) {
        if (bucket.is_array()) {
            if (handle < bucket.size()) {
                out = bucket.at(handle);
                return true;
            }
            return false;
        }

        const std::string key = std::to_string(handle);
        if (bucket.contains(key)) {
            out = bucket.at(key);
            return true;
        }
        return false;
    }

    inline void set_property_value_for_handle(nlohmann::json &bucket,
                                              const unsigned handle,
                                              const nlohmann::json &value) {
        if (bucket.is_array()) {
            while (bucket.size() <= handle) {
                bucket.push_back(nullptr);
            }
            bucket[handle] = value;
            return;
        }
        bucket[std::to_string(handle)] = value;
    }

    inline nlohmann::json remap_property_store(const nlohmann::json &store,
                                               const std::unordered_map<unsigned, unsigned> &handle_remap) {
        nlohmann::json out = nlohmann::json::object();
        if (!store.is_object()) {
            return out;
        }

        for (auto it = store.begin(); it != store.end(); ++it) {
            nlohmann::json remapped_bucket = nlohmann::json::object();
            for (const auto &[old_handle, new_handle]: handle_remap) {
                nlohmann::json value;
                if (try_get_property_value_for_handle(it.value(), old_handle, value)) {
                    set_property_value_for_handle(remapped_bucket, new_handle, value);
                }
            }
            out[it.key()] = std::move(remapped_bucket);
        }
        return out;
    }

    inline nlohmann::json interpolate_property_value(const nlohmann::json &a,
                                                     const nlohmann::json &b,
                                                     const double t) {
        if (a.is_number() && b.is_number()) {
            const double av = a.get<double>();
            const double bv = b.get<double>();
            return (1.0 - t) * av + t * bv;
        }
        if (a == b) {
            return a;
        }
        return (t < 0.5) ? a : b;
    }

    inline std::unordered_set<unsigned> collect_vertex_handles(const std::vector<vertexPtr> &vertices) {
        std::unordered_set<unsigned> out;
        out.reserve(vertices.size());
        for (const auto &v: vertices) {
            if (v) {
                out.insert(v->get_handle());
            }
        }
        return out;
    }

    inline std::unordered_set<unsigned> collect_edge_handles(const std::vector<edgePtr> &edges) {
        std::unordered_set<unsigned> out;
        out.reserve(edges.size());
        for (const auto &e: edges) {
            if (e) {
                out.insert(e->get_handle());
            }
        }
        return out;
    }

    inline std::unordered_set<unsigned> collect_face_handles(const std::vector<facePtr> &faces) {
        std::unordered_set<unsigned> out;
        out.reserve(faces.size());
        for (const auto &f: faces) {
            if (f) {
                out.insert(f->get_handle());
            }
        }
        return out;
    }

    inline std::vector<unsigned> set_difference(const std::unordered_set<unsigned> &after,
                                                const std::unordered_set<unsigned> &before) {
        std::vector<unsigned> out;
        for (const auto h: after) {
            if (!before.count(h)) {
                out.push_back(h);
            }
        }
        std::sort(out.begin(), out.end());
        return out;
    }
} // namespace halfMesh::detail
