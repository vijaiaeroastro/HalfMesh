#include "triMesh.hpp"
#include "mesh_edit_common.hpp"

namespace halfMesh {
    using detail::collect_edge_handles;
    using detail::collect_face_handles;
    using detail::collect_vertex_handles;
    using detail::face_contains_handle;
    using detail::get_oriented_face_vertices;
    using detail::make_edge_key;
    using detail::make_face_key;
    using detail::remap_property_store;
    using detail::set_difference;
    using detail::to_handle_triplet;

    bool triMesh::can_collapse(const edgePtr &e, const vertexPtr &target) const {
        if (!e || !target) {
            return false;
        }
        if (!has_edge_handle(e->get_handle()) || get_edge(e->get_handle()) != e) {
            return false;
        }
        if (!has_vertex_handle(target->get_handle()) || get_vertex(target->get_handle()) != target) {
            return false;
        }

        const auto v0 = e->get_vertex_one();
        const auto v1 = e->get_vertex_two();
        if (!v0 || !v1 || v0->get_handle() == v1->get_handle()) {
            return false;
        }

        const auto target_h = target->get_handle();
        if (target_h != v0->get_handle() && target_h != v1->get_handle()) {
            return false;
        }
        const auto source_h = (target_h == v0->get_handle()) ? v1->get_handle() : v0->get_handle();

        // Link condition: common one-ring neighbors of both endpoints must be exactly
        // the opposite vertices of faces incident to this edge.
        std::unordered_set<unsigned> opposite_handles;
        if (const auto he0 = e->get_one_half_edge()) {
            if (const auto f = he0->get_parent_face()) {
                const auto verts = get_oriented_face_vertices(f);
                for (const auto &v: verts) {
                    if (!v) continue;
                    const auto h = v->get_handle();
                    if (h != v0->get_handle() && h != v1->get_handle()) {
                        opposite_handles.insert(h);
                    }
                }
            }
            if (const auto opp = he0->get_opposing_half_edge()) {
                if (const auto f = opp->get_parent_face()) {
                    const auto verts = get_oriented_face_vertices(f);
                    for (const auto &v: verts) {
                        if (!v) continue;
                        const auto h = v->get_handle();
                        if (h != v0->get_handle() && h != v1->get_handle()) {
                            opposite_handles.insert(h);
                        }
                    }
                }
            }
        }
        if (opposite_handles.empty() || opposite_handles.size() > 2) {
            return false;
        }

        std::unordered_set<unsigned> n0;
        for (const auto &vn: one_ring_vertex_of_a_vertex(v0)) {
            if (vn) n0.insert(vn->get_handle());
        }
        std::unordered_set<unsigned> n1;
        for (const auto &vn: one_ring_vertex_of_a_vertex(v1)) {
            if (vn) n1.insert(vn->get_handle());
        }
        n0.erase(v1->get_handle());
        n1.erase(v0->get_handle());

        std::unordered_set<unsigned> common_neighbors;
        for (const auto h: n0) {
            if (n1.count(h)) {
                common_neighbors.insert(h);
            }
        }
        if (common_neighbors != opposite_handles) {
            return false;
        }

        std::unordered_set<FaceKey, FaceKeyHash, FaceKeyEqual> future_faces;

        for (const auto &f: faces_) {
            const auto verts = get_oriented_face_vertices(f);
            if (!verts[0] || !verts[1] || !verts[2]) {
                return false;
            }

            const bool has_target = face_contains_handle(verts, target_h);
            const bool has_source = face_contains_handle(verts, source_h);

            // Faces incident to the collapsed edge are removed.
            if (has_target && has_source) {
                continue;
            }

            auto h = to_handle_triplet(verts);
            for (auto &x: h) {
                if (x == source_h) {
                    x = target_h;
                }
            }

            // Any non-removed face becoming degenerate is invalid.
            if (h[0] == h[1] || h[1] == h[2] || h[2] == h[0]) {
                return false;
            }

            const FaceKey fk = make_face_key(h[0], h[1], h[2]);
            if (!future_faces.insert(fk).second) {
                return false;
            }
        }

        return true;
    }

    EditResult triMesh::collapse_edge(const edgePtr &e, const vertexPtr &target) {
        EditResult result{};

        if (!can_collapse(e, target)) {
            result.error = "Edge collapse preconditions failed.";
            return result;
        }

        const auto v0 = e->get_vertex_one();
        const auto v1 = e->get_vertex_two();
        const auto target_h = target->get_handle();
        const auto source_h = (target_h == v0->get_handle()) ? v1->get_handle() : v0->get_handle();
        const auto source_vertex = (target_h == v0->get_handle()) ? v1 : v0;

        const auto before_vertices = collect_vertex_handles(vertices_);
        const auto before_edges = collect_edge_handles(edges_);
        const auto before_faces = collect_face_handles(faces_);
        const nlohmann::json old_vertex_properties = vertex_data_store;
        const nlohmann::json old_edge_properties = edge_data_store;
        const nlohmann::json old_face_properties = face_data_store;

        struct OldEdgeInfo {
            unsigned handle = 0;
            unsigned v1 = 0;
            unsigned v2 = 0;
        };
        std::vector<OldEdgeInfo> old_edges;
        old_edges.reserve(edges_.size());
        for (const auto &old_edge: edges_) {
            const auto ov1 = old_edge->get_vertex_one();
            const auto ov2 = old_edge->get_vertex_two();
            if (!ov1 || !ov2) {
                continue;
            }
            old_edges.push_back({old_edge->get_handle(), ov1->get_handle(), ov2->get_handle()});
        }

        struct FaceCollapsePlan {
            facePtr old_face;
            unsigned old_face_handle = 0;
            bool remove_only = false;
            std::array<vertexPtr, 3> new_vertices{nullptr, nullptr, nullptr};
        };
        std::vector<FaceCollapsePlan> plans;
        plans.reserve(faces_.size());

        for (const auto &f: faces_) {
            const auto verts = get_oriented_face_vertices(f);
            const bool has_target = face_contains_handle(verts, target_h);
            const bool has_source = face_contains_handle(verts, source_h);

            if (!has_source) {
                continue;
            }

            FaceCollapsePlan plan{};
            plan.old_face = f;
            plan.old_face_handle = f->get_handle();
            plan.remove_only = has_target;
            if (!plan.remove_only) {
                plan.new_vertices = verts;
                for (auto &v: plan.new_vertices) {
                    if (v && v->get_handle() == source_h) {
                        v = target;
                    }
                }
                if (!plan.new_vertices[0] || !plan.new_vertices[1] || !plan.new_vertices[2]) {
                    result.error = "Invalid face vertex data during collapse.";
                    return result;
                }
                const auto h0 = plan.new_vertices[0]->get_handle();
                const auto h1 = plan.new_vertices[1]->get_handle();
                const auto h2 = plan.new_vertices[2]->get_handle();
                if (h0 == h1 || h1 == h2 || h2 == h0) {
                    result.error = "Collapse would create a degenerate face.";
                    return result;
                }
            }
            plans.push_back(plan);
        }

        for (const auto &plan: plans) {
            if (!delete_face(plan.old_face)) {
                result.error = "Failed to delete incident face during collapse.";
                return result;
            }
            result.removed_faces.push_back(plan.old_face_handle);
        }

        if (!delete_vertex(source_vertex)) {
            result.error = "Failed to delete collapsed source vertex.";
            return result;
        }
        result.removed_vertices.push_back(source_h);

        for (const auto &plan: plans) {
            if (plan.remove_only) {
                continue;
            }
            const auto new_face = add_face(plan.new_vertices[0], plan.new_vertices[1], plan.new_vertices[2]);
            if (!new_face) {
                result.error = "Failed to create rewired face during collapse.";
                return result;
            }
            result.face_handle_remap[plan.old_face_handle] = new_face->get_handle();
        }

        const auto after_vertices = collect_vertex_handles(vertices_);
        const auto after_edges = collect_edge_handles(edges_);
        const auto after_faces = collect_face_handles(faces_);
        result.created_vertices = set_difference(after_vertices, before_vertices);
        result.created_edges = set_difference(after_edges, before_edges);
        result.created_faces = set_difference(after_faces, before_faces);
        const auto removed_vertices = set_difference(before_vertices, after_vertices);
        result.removed_vertices.insert(result.removed_vertices.end(), removed_vertices.begin(), removed_vertices.end());
        const auto removed_edges = set_difference(before_edges, after_edges);
        result.removed_edges.insert(result.removed_edges.end(), removed_edges.begin(), removed_edges.end());
        const auto removed_faces = set_difference(before_faces, after_faces);
        result.removed_faces.insert(result.removed_faces.end(), removed_faces.begin(), removed_faces.end());

        auto sort_unique = [](std::vector<unsigned> &v) {
            std::sort(v.begin(), v.end());
            v.erase(std::unique(v.begin(), v.end()), v.end());
        };
        sort_unique(result.created_vertices);
        sort_unique(result.created_edges);
        sort_unique(result.created_faces);
        sort_unique(result.removed_vertices);
        sort_unique(result.removed_edges);
        sort_unique(result.removed_faces);

        for (const auto h: before_vertices) {
            if (after_vertices.count(h)) {
                result.vertex_handle_remap[h] = h;
            }
        }
        for (const auto h: before_edges) {
            if (after_edges.count(h)) {
                result.edge_handle_remap[h] = h;
            }
        }
        for (const auto h: before_faces) {
            if (after_faces.count(h) && !result.face_handle_remap.count(h)) {
                result.face_handle_remap[h] = h;
            }
        }

        std::unordered_map<EdgeKey, unsigned, EdgeKeyHash, EdgeKeyEqual> new_edge_by_key;
        for (const auto &new_edge: edges_) {
            const auto nv1 = new_edge->get_vertex_one();
            const auto nv2 = new_edge->get_vertex_two();
            if (!nv1 || !nv2) {
                continue;
            }
            new_edge_by_key[make_edge_key(nv1->get_handle(), nv2->get_handle())] = new_edge->get_handle();
        }

        for (const auto &old_edge: old_edges) {
            auto ov1 = old_edge.v1;
            auto ov2 = old_edge.v2;
            if (ov1 == source_h) ov1 = target_h;
            if (ov2 == source_h) ov2 = target_h;
            const auto it1 = result.vertex_handle_remap.find(ov1);
            const auto it2 = result.vertex_handle_remap.find(ov2);
            if (it1 == result.vertex_handle_remap.end() || it2 == result.vertex_handle_remap.end()) {
                continue;
            }
            if (it1->second == it2->second) {
                continue;
            }
            const EdgeKey new_key = make_edge_key(it1->second, it2->second);
            if (const auto neit = new_edge_by_key.find(new_key); neit != new_edge_by_key.end()) {
                result.edge_handle_remap[old_edge.handle] = neit->second;
            }
        }

        vertex_data_store = remap_property_store(old_vertex_properties, result.vertex_handle_remap);
        edge_data_store = remap_property_store(old_edge_properties, result.edge_handle_remap);
        face_data_store = remap_property_store(old_face_properties, result.face_handle_remap);

        result.ok = true;
        return result;
    }
} // namespace halfMesh
