#include "triMesh.hpp"
#include "mesh_edit_common.hpp"

namespace halfMesh {
    using detail::collect_edge_handles;
    using detail::collect_face_handles;
    using detail::collect_vertex_handles;
    using detail::get_oriented_face_vertices;
    using detail::make_edge_key;
    using detail::make_face_key;
    using detail::remap_property_store;
    using detail::set_difference;
    using detail::set_property_value_for_handle;
    using detail::try_get_property_value_for_handle;

    bool triMesh::can_flip(const edgePtr &e) const {
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
        const auto opp = he0->get_opposing_half_edge();
        if (!opp) {
            return false; // boundary edge
        }

        const auto f0 = he0->get_parent_face();
        const auto f1 = opp->get_parent_face();
        if (!f0 || !f1 || f0->get_handle() == f1->get_handle()) {
            return false;
        }

        const auto get_opposite = [&](const facePtr &f) -> vertexPtr {
            const auto verts = get_oriented_face_vertices(f);
            for (const auto &v: verts) {
                if (!v) {
                    continue;
                }
                const auto h = v->get_handle();
                if (h != a->get_handle() && h != b->get_handle()) {
                    return v;
                }
            }
            return nullptr;
        };

        const auto c = get_opposite(f0);
        const auto d = get_opposite(f1);
        if (!c || !d) {
            return false;
        }
        if (c->get_handle() == d->get_handle()) {
            return false;
        }

        // New diagonal must not already exist.
        if (const auto it = edge_lookup_.find(make_edge_key(c->get_handle(), d->get_handle()));
            it != edge_lookup_.end()) {
            return false;
        }

        const auto a_h = a->get_handle();
        const auto b_h = b->get_handle();
        const auto c_h = c->get_handle();
        const auto d_h = d->get_handle();
        const auto fk1 = make_face_key(c_h, d_h, b_h);
        const auto fk2 = make_face_key(d_h, c_h, a_h);
        if (fk1 == fk2) {
            return false;
        }

        const auto f0_h = f0->get_handle();
        const auto f1_h = f1->get_handle();
        const auto face_conflicts = [&](const FaceKey &fk) {
            const auto it = face_lookup_.find(fk);
            return it != face_lookup_.end() && it->second != f0_h && it->second != f1_h;
        };

        if (face_conflicts(fk1) || face_conflicts(fk2)) {
            return false;
        }

        return true;
    }

    EditResult triMesh::flip_edge(const edgePtr &e) {
        EditResult result{};

        if (!can_flip(e)) {
            result.error = "Edge flip preconditions failed.";
            return result;
        }

        const auto a = e->get_vertex_one();
        const auto b = e->get_vertex_two();
        const auto flipped_edge_handle = e->get_handle();
        const auto he0 = e->get_one_half_edge();
        const auto opp = he0->get_opposing_half_edge();
        const auto f0 = he0->get_parent_face();
        const auto f1 = opp->get_parent_face();
        const auto f0_handle = f0->get_handle();
        const auto f1_handle = f1->get_handle();

        const nlohmann::json old_vertex_properties = vertex_data_store;
        const nlohmann::json old_edge_properties = edge_data_store;
        const nlohmann::json old_face_properties = face_data_store;

        const auto get_opposite = [&](const facePtr &f) -> vertexPtr {
            const auto verts = get_oriented_face_vertices(f);
            for (const auto &v: verts) {
                if (!v) {
                    continue;
                }
                const auto h = v->get_handle();
                if (h != a->get_handle() && h != b->get_handle()) {
                    return v;
                }
            }
            return nullptr;
        };

        const auto c = get_opposite(f0);
        const auto d = get_opposite(f1);
        if (!c || !d) {
            result.error = "Could not determine opposite vertices for flip.";
            return result;
        }

        const auto before_vertices = collect_vertex_handles(vertices_);
        const auto before_edges = collect_edge_handles(edges_);
        const auto before_faces = collect_face_handles(faces_);

        result.removed_faces = {f0_handle, f1_handle};
        result.removed_edges = {flipped_edge_handle};

        if (!delete_face(f0) || !delete_face(f1)) {
            result.error = "Failed to delete source faces during flip.";
            return result;
        }
        if (!delete_edge(e)) {
            result.error = "Failed to delete source edge during flip.";
            return result;
        }

        const auto new_f0 = add_face(c, d, b);
        const auto new_f1 = add_face(d, c, a);
        if (!new_f0 || !new_f1) {
            result.error = "Failed to create flipped faces.";
            return result;
        }
        result.face_handle_remap[f0_handle] = new_f0->get_handle();
        result.face_handle_remap[f1_handle] = new_f1->get_handle();
        result.created_faces = {new_f0->get_handle(), new_f1->get_handle()};
        std::sort(result.created_faces.begin(), result.created_faces.end());

        const auto new_cd_edge = edge_lookup_.find(make_edge_key(c->get_handle(), d->get_handle()));
        if (new_cd_edge == edge_lookup_.end()) {
            result.error = "Failed to locate new diagonal edge after flip.";
            return result;
        }
        result.edge_handle_remap[flipped_edge_handle] = new_cd_edge->second;
        result.created_edges = {new_cd_edge->second};

        // Local flip keeps vertex handles unchanged.
        for (const auto h: before_vertices) {
            if (has_vertex_handle(h)) {
                result.vertex_handle_remap[h] = h;
            }
        }

        // Preserve unchanged edge/face remaps as identity.
        for (const auto h: before_edges) {
            if (h != flipped_edge_handle && has_edge_handle(h)) {
                result.edge_handle_remap[h] = h;
            }
        }
        for (const auto h: before_faces) {
            if (h != f0_handle && h != f1_handle && has_face_handle(h)) {
                result.face_handle_remap[h] = h;
            }
        }

        vertex_data_store = remap_property_store(old_vertex_properties, result.vertex_handle_remap);
        edge_data_store = remap_property_store(old_edge_properties, result.edge_handle_remap);
        face_data_store = remap_property_store(old_face_properties, result.face_handle_remap);

        // Keep explicit property inheritance for changed entities.
        for (auto it = old_face_properties.begin(); it != old_face_properties.end(); ++it) {
            nlohmann::json v0prop;
            nlohmann::json v1prop;
            if (try_get_property_value_for_handle(it.value(), f0_handle, v0prop)) {
                set_property_value_for_handle(face_data_store[it.key()], new_f0->get_handle(), v0prop);
            }
            if (try_get_property_value_for_handle(it.value(), f1_handle, v1prop)) {
                set_property_value_for_handle(face_data_store[it.key()], new_f1->get_handle(), v1prop);
            }
        }
        for (auto it = old_edge_properties.begin(); it != old_edge_properties.end(); ++it) {
            nlohmann::json eprop;
            if (try_get_property_value_for_handle(it.value(), flipped_edge_handle, eprop)) {
                set_property_value_for_handle(edge_data_store[it.key()], new_cd_edge->second, eprop);
            }
        }

        const auto after_edges = collect_edge_handles(edges_);
        const auto after_faces = collect_face_handles(faces_);
        const auto removed_edges = set_difference(before_edges, after_edges);
        for (const auto h: removed_edges) {
            if (std::find(result.removed_edges.begin(), result.removed_edges.end(), h) == result.removed_edges.end()) {
                result.removed_edges.push_back(h);
            }
        }
        const auto removed_faces = set_difference(before_faces, after_faces);
        for (const auto h: removed_faces) {
            if (std::find(result.removed_faces.begin(), result.removed_faces.end(), h) == result.removed_faces.end()) {
                result.removed_faces.push_back(h);
            }
        }

        result.ok = true;
        return result;
    }
}
