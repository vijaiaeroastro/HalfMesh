#include "triMesh.hpp"
#include "mesh_edit_common.hpp"

namespace halfMesh {
    using detail::collect_edge_handles;
    using detail::collect_face_handles;
    using detail::collect_vertex_handles;
    using detail::get_oriented_face_vertices;
    using detail::interpolate_property_value;
    using detail::remap_property_store;
    using detail::set_difference;
    using detail::set_property_value_for_handle;
    using detail::try_get_property_value_for_handle;

    bool triMesh::can_split(const edgePtr &e) const {
        if (!e) {
            return false;
        }
        if (!has_edge_handle(e->get_handle()) || get_edge(e->get_handle()) != e) {
            return false;
        }

        const auto a = e->get_vertex_one();
        const auto b = e->get_vertex_two();
        if (!a || !b || a->get_handle() == b->get_handle()) {
            return false;
        }

        const auto he0 = e->get_one_half_edge();
        if (!he0) {
            return false;
        }

        std::vector<facePtr> incident_faces;
        if (const auto f = he0->get_parent_face()) {
            incident_faces.push_back(f);
        }
        if (const auto opp = he0->get_opposing_half_edge()) {
            if (const auto f = opp->get_parent_face();
                f && (incident_faces.empty() || incident_faces.front()->get_handle() != f->get_handle())) {
                incident_faces.push_back(f);
            }
        }

        if (incident_faces.empty() || incident_faces.size() > 2) {
            return false;
        }

        for (const auto &f: incident_faces) {
            if (!f) {
                return false;
            }
            const auto verts = get_oriented_face_vertices(f);
            if (!verts[0] || !verts[1] || !verts[2]) {
                return false;
            }

            int a_count = 0;
            int b_count = 0;
            for (const auto &v: verts) {
                if (v->get_handle() == a->get_handle()) {
                    ++a_count;
                }
                if (v->get_handle() == b->get_handle()) {
                    ++b_count;
                }
            }
            if (a_count != 1 || b_count != 1) {
                return false;
            }

            bool found_adjacent_edge = false;
            for (int i = 0; i < 3; ++i) {
                const auto v_curr = verts[i];
                const auto v_next = verts[(i + 1) % 3];
                if (!v_curr || !v_next) {
                    return false;
                }
                const auto hc = v_curr->get_handle();
                const auto hn = v_next->get_handle();
                const bool matches_ab = hc == a->get_handle() && hn == b->get_handle();
                const bool matches_ba = hc == b->get_handle() && hn == a->get_handle();
                if (matches_ab || matches_ba) {
                    found_adjacent_edge = true;
                    break;
                }
            }
            if (!found_adjacent_edge) {
                return false;
            }
        }

        return true;
    }

    EditResult triMesh::split_edge(const edgePtr &e, const double t) {
        EditResult result{};

        if (!(t > 0.0 && t < 1.0)) {
            result.error = "Split parameter t must satisfy 0 < t < 1.";
            return result;
        }

        if (!can_split(e)) {
            result.error = "Edge does not satisfy split preconditions.";
            return result;
        }

        const auto a = e->get_vertex_one();
        const auto b = e->get_vertex_two();
        const unsigned edge_handle = e->get_handle();
        const unsigned a_handle = a->get_handle();
        const unsigned b_handle = b->get_handle();

        const auto before_vertices = collect_vertex_handles(vertices_);
        const auto before_edges = collect_edge_handles(edges_);
        const auto before_faces = collect_face_handles(faces_);

        const nlohmann::json old_vertex_properties = vertex_data_store;
        const nlohmann::json old_edge_properties = edge_data_store;
        const nlohmann::json old_face_properties = face_data_store;

        const double mx = (1.0 - t) * a->get_x() + t * b->get_x();
        const double my = (1.0 - t) * a->get_y() + t * b->get_y();
        const double mz = (1.0 - t) * a->get_z() + t * b->get_z();

        struct FaceSplitPlan {
            facePtr face_ref;
            vertexPtr u;
            vertexPtr v;
            vertexPtr w;
            unsigned face_handle = 0;
        };

        std::vector<FaceSplitPlan> plans;
        if (const auto he0 = e->get_one_half_edge()) {
            if (const auto f = he0->get_parent_face()) {
                plans.push_back({f, nullptr, nullptr, nullptr});
            }
            if (const auto opp = he0->get_opposing_half_edge()) {
                if (const auto f = opp->get_parent_face();
                    f && std::none_of(plans.begin(), plans.end(), [&f](const FaceSplitPlan &p) {
                        return p.face_ref && p.face_ref->get_handle() == f->get_handle();
                    })) {
                    plans.push_back({f, nullptr, nullptr, nullptr});
                }
            }
        }

        if (plans.empty() || plans.size() > 2) {
            result.error = "Split edge must have one or two incident faces.";
            return result;
        }

        for (auto &plan: plans) {
            const auto verts = get_oriented_face_vertices(plan.face_ref);
            bool found = false;
            for (int i = 0; i < 3; ++i) {
                const auto u = verts[i];
                const auto v = verts[(i + 1) % 3];
                const auto w = verts[(i + 2) % 3];
                if (!u || !v || !w) {
                    result.error = "Incident face has invalid vertex data.";
                    return result;
                }
                const bool matches_ab = u->get_handle() == a->get_handle() && v->get_handle() == b->get_handle();
                const bool matches_ba = u->get_handle() == b->get_handle() && v->get_handle() == a->get_handle();
                if (matches_ab || matches_ba) {
                    plan.u = u;
                    plan.v = v;
                    plan.w = w;
                    plan.face_handle = plan.face_ref->get_handle();
                    found = true;
                    break;
                }
            }
            if (!found || !plan.u || !plan.v || !plan.w) {
                result.error = "Could not build split plan for incident face.";
                return result;
            }
        }

        for (const auto &plan: plans) {
            if (!delete_face(plan.face_ref)) {
                result.error = "Failed to delete incident face during split.";
                return result;
            }
            result.removed_faces.push_back(plan.face_handle);
        }
        if (!delete_edge(e)) {
            result.error = "Failed to delete source edge during split.";
            return result;
        }
        result.removed_edges.push_back(edge_handle);

        const auto mid = add_vertex(mx, my, mz);
        if (!mid) {
            result.error = "Failed to create split vertex.";
            return result;
        }
        result.created_vertices.push_back(mid->get_handle());

        std::unordered_map<unsigned, std::vector<unsigned> > face_split_children;
        for (const auto &plan: plans) {
            const auto f1 = add_face(plan.u, mid, plan.w);
            if (!f1) {
                result.error = "Failed to create first child face during split.";
                return result;
            }
            const auto f2 = add_face(mid, plan.v, plan.w);
            if (!f2) {
                result.error = "Failed to create second child face during split.";
                return result;
            }
            face_split_children[plan.face_handle] = {f1->get_handle(), f2->get_handle()};
        }

        const auto after_vertices = collect_vertex_handles(vertices_);
        const auto after_edges = collect_edge_handles(edges_);
        const auto after_faces = collect_face_handles(faces_);
        result.created_edges = set_difference(after_edges, before_edges);
        const auto removed_vertices = set_difference(before_vertices, after_vertices);
        result.removed_vertices.insert(result.removed_vertices.end(), removed_vertices.begin(), removed_vertices.end());
        const auto removed_edges = set_difference(before_edges, after_edges);
        for (const auto h: removed_edges) {
            if (std::find(result.removed_edges.begin(), result.removed_edges.end(), h) == result.removed_edges.end()) {
                result.removed_edges.push_back(h);
            }
        }
        result.created_faces = set_difference(after_faces, before_faces);

        // Local split keeps unaffected handles stable.
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
            if (after_faces.count(h)) {
                result.face_handle_remap[h] = h;
            }
        }

        // Rebuild property stores for surviving entities to drop removed-handle entries.
        vertex_data_store = remap_property_store(old_vertex_properties, result.vertex_handle_remap);
        edge_data_store = remap_property_store(old_edge_properties, result.edge_handle_remap);
        face_data_store = remap_property_store(old_face_properties, result.face_handle_remap);

        // Vertex property propagation: interpolate source edge endpoint values to the split vertex.
        for (auto it = old_vertex_properties.begin(); it != old_vertex_properties.end(); ++it) {
            nlohmann::json av;
            nlohmann::json bv;
            if (try_get_property_value_for_handle(it.value(), a_handle, av) &&
                try_get_property_value_for_handle(it.value(), b_handle, bv)) {
                const auto blended = interpolate_property_value(av, bv, t);
                set_property_value_for_handle(vertex_data_store[it.key()], mid->get_handle(), blended);
            }
        }

        // Face property propagation: each split child inherits its parent face properties.
        for (const auto &[old_face_handle, new_face_handles]: face_split_children) {
            for (auto it = old_face_properties.begin(); it != old_face_properties.end(); ++it) {
                nlohmann::json old_value;
                if (!try_get_property_value_for_handle(it.value(), old_face_handle, old_value)) {
                    continue;
                }
                for (const auto new_face_handle: new_face_handles) {
                    set_property_value_for_handle(face_data_store[it.key()], new_face_handle, old_value);
                }
            }
        }

        // Edge property propagation: children on the original edge inherit source edge properties.
        for (auto it = old_edge_properties.begin(); it != old_edge_properties.end(); ++it) {
            nlohmann::json old_edge_value;
            if (!try_get_property_value_for_handle(it.value(), edge_handle, old_edge_value)) {
                continue;
            }
            for (const auto new_edge_handle: result.created_edges) {
                const auto new_edge = get_edge(new_edge_handle);
                if (!new_edge) {
                    continue;
                }
                const auto ev1 = new_edge->get_vertex_one();
                const auto ev2 = new_edge->get_vertex_two();
                if (!ev1 || !ev2) {
                    continue;
                }
                const bool touches_mid = ev1->get_handle() == mid->get_handle() || ev2->get_handle() == mid->get_handle();
                const bool touches_original_endpoint =
                    ev1->get_handle() == a_handle || ev1->get_handle() == b_handle ||
                    ev2->get_handle() == a_handle || ev2->get_handle() == b_handle;
                if (touches_mid && touches_original_endpoint) {
                    set_property_value_for_handle(edge_data_store[it.key()], new_edge_handle, old_edge_value);
                }
            }
        }

        result.ok = true;
        return result;
    }
} // namespace halfMesh
