#include <iostream>

#include "triMesh.hpp"
#include <utility>      // for std::swap

namespace halfMesh {
    // Canonicalization helpers (could also live in a detail header)
    inline EdgeKey make_edge_key(unsigned i, unsigned j) {
        if (i > j) std::swap(i, j);
        return {i, j};
    }

    inline FaceKey make_face_key(unsigned a, unsigned b, unsigned c) {
        std::array<unsigned, 3> v{a, b, c};
        std::sort(v.begin(), v.end());
        return {v[0], v[1], v[2]};
    }

    std::array<vertexPtr, 3> get_oriented_face_vertices(const facePtr &f) {
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

    bool face_contains_handle(const std::array<vertexPtr, 3> &verts, const unsigned h) {
        for (const auto &v: verts) {
            if (v && v->get_handle() == h) {
                return true;
            }
        }
        return false;
    }

    std::array<unsigned, 3> to_handle_triplet(const std::array<vertexPtr, 3> &verts) {
        return {
            verts[0]->get_handle(),
            verts[1]->get_handle(),
            verts[2]->get_handle()
        };
    }

    triMesh::triMesh() = default;

    triMesh::~triMesh() = default;

    void triMesh::clear_data() {
        vertices_.clear();
        half_edges_.clear();
        edges_.clear();
        faces_.clear();
        handle_to_vertex_.clear();
        handle_to_half_edge_.clear();
        handle_to_edge_.clear();
        handle_to_face_.clear();
        edge_lookup_.clear();
        face_lookup_.clear();
        half_edge_lookup_.clear();
        vertex_data_store.clear();
        edge_data_store.clear();
        face_data_store.clear();
        next_vertex_handle_ = 0;
        next_half_edge_handle_ = 0;
        next_edge_handle_ = 0;
        next_face_handle_ = 0;
        topology_dirty_ = false;
    }

    // Core mutators
    vertexPtr triMesh::add_vertex(double x, double y, double z) {
        auto v = std::make_shared<vertex>(x, y, z);
        unsigned h = next_vertex_handle_++;
        v->set_handle(h);
        vertices_.push_back(v);
        handle_to_vertex_[h] = v;
        topology_dirty_ = true;
        // std::cout << "Added vertex : " << h << " with coordinates : " << x << "," << y << "," << z << std::endl;
        return v;
    }

    halfEdgePtr triMesh::add_half_edge(const vertexPtr &v1,
                                       const vertexPtr &v2,
                                       const facePtr &f) {
        const HalfEdgeKey key{v1->get_handle(), v2->get_handle()};
        if (const auto it = half_edge_lookup_.find(key); it != half_edge_lookup_.end())
            return it->second;

        auto he = std::make_shared<halfedge>(v1, v2);
        const unsigned h = next_half_edge_handle_++;
        he->set_handle(h);
        he->set_parent_face(f);

        // link opposites
        const auto rev = std::make_pair(v2->get_handle(), v1->get_handle());
        if (const auto rit = half_edge_lookup_.find(rev); rit != half_edge_lookup_.end()) {
            const auto opp = rit->second;
            he->set_opposing_half_edge(opp);
            opp->set_opposing_half_edge(he);
        }

        v1->add_outgoing_half_edge(he);
        v2->add_incoming_half_edge(he);

        half_edges_.push_back(he);
        handle_to_half_edge_[h] = he;
        half_edge_lookup_[key] = he;
        topology_dirty_ = true;
        return he;
    }

    edgePtr triMesh::add_edge(const vertexPtr &v1,
                              const vertexPtr &v2,
                              const facePtr &f) {
        const auto key = make_edge_key(v1->get_handle(), v2->get_handle());
        if (const auto it = edge_lookup_.find(key); it != edge_lookup_.end()) {
            const auto e = handle_to_edge_[it->second];
            const auto he = add_half_edge(v1, v2, f);
            he->set_parent_edge(e);
            e->set_one_half_edge(he);
            topology_dirty_ = true;
            // std::cout << "----> Found an existing edge : "
            // << e->handle() << " between " << e->get_vertex_one()->handle() << "," << e->get_vertex_two()->handle() << std::endl;
            return e;
        }

        auto e = std::make_shared<edge>(v1, v2);
        const unsigned h = next_edge_handle_++;
        e->set_handle(h);
        edges_.push_back(e);
        handle_to_edge_[h] = e;
        edge_lookup_[key] = h;

        const auto he = add_half_edge(v1, v2, f);
        he->set_parent_edge(e);
        e->set_one_half_edge(he);
        topology_dirty_ = true;

        // std::cout << "----> Created a new edge : "
        // << e->handle() << " between " << e->get_vertex_one()->handle() << "," << e->get_vertex_two()->handle() << std::endl;
        return e;
    }

    facePtr triMesh::add_face(const vertexPtr &v1,
                          const vertexPtr &v2,
                          const vertexPtr &v3) {
        if (!v1 || !v2 || !v3) {
            return nullptr;
        }

        const auto h1 = v1->get_handle();
        const auto h2 = v2->get_handle();
        const auto h3 = v3->get_handle();

        // Degenerate triangles are invalid in this mesh representation.
        if (h1 == h2 || h2 == h3 || h3 == h1) {
            return nullptr;
        }

        if (!has_vertex_handle(h1) || !has_vertex_handle(h2) || !has_vertex_handle(h3)) {
            return nullptr;
        }

        const auto key = make_face_key(v1->get_handle(),
                                       v2->get_handle(),
                                       v3->get_handle());
        if (const auto it = face_lookup_.find(key); it != face_lookup_.end()) {
            return handle_to_face_[it->second];
        }

        // Reject inserts that would reuse an already-owned directed half-edge.
        const HalfEdgeKey k12{h1, h2};
        const HalfEdgeKey k23{h2, h3};
        const HalfEdgeKey k31{h3, h1};
        const auto conflicts_with_existing_face = [this](const HalfEdgeKey &k) {
            const auto it = half_edge_lookup_.find(k);
            return it != half_edge_lookup_.end() && it->second && it->second->get_parent_face();
        };
        if (conflicts_with_existing_face(k12) ||
            conflicts_with_existing_face(k23) ||
            conflicts_with_existing_face(k31)) {
            return nullptr;
        }

        // 1) create the Face
        auto f = std::make_shared<face>(v1, v2, v3);
        const unsigned fh = next_face_handle_++;
        f->set_handle(fh);
        faces_.push_back(f);
        handle_to_face_[fh] = f;
        face_lookup_[key] = fh;

        // 2) create (or reuse) the three half‐edges via add_edge()
        //    add_edge will call add_half_edge under the hood
        const auto e1 = add_edge(v1, v2, f);
        const auto e2 = add_edge(v2, v3, f);
        const auto e3 = add_edge(v3, v1, f);

        // 3) Pull out the directed half-edges for this face.
        const auto he_it_12 = half_edge_lookup_.find(k12);
        const auto he_it_23 = half_edge_lookup_.find(k23);
        const auto he_it_31 = half_edge_lookup_.find(k31);
        if (he_it_12 == half_edge_lookup_.end() ||
            he_it_23 == half_edge_lookup_.end() ||
            he_it_31 == half_edge_lookup_.end()) {
            // Roll back the face if the face cycle cannot be built.
            face_lookup_.erase(key);
            handle_to_face_.erase(fh);
            faces_.pop_back();
            return nullptr;
        }
        const auto he1 = he_it_12->second;
        const auto he2 = he_it_23->second;
        const auto he3 = he_it_31->second;
        he1->set_parent_face(f);
        he2->set_parent_face(f);
        he3->set_parent_face(f);

        // 4) link them into a ccw cycle around the face
        he1->set_next(he2);
        he2->set_next(he3);
        he3->set_next(he1);

        he1->set_prev(he3);
        he2->set_prev(he1);
        he3->set_prev(he2);

        // 5) store one representative half‐edge on f
        f->set_one_half_edge(he1);
        topology_dirty_ = true;

        return f;
    }

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

    vertexPtr triMesh::split_edge(const edgePtr &e) {
        return split_edge(e, 0.5);
    }

    vertexPtr triMesh::split_edge(const edgePtr &e, const double t) {
        if (!(t > 0.0 && t < 1.0)) {
            return nullptr;
        }

        if (!can_split(e)) {
            return nullptr;
        }

        const auto a = e->get_vertex_one();
        const auto b = e->get_vertex_two();
        const double mx = (1.0 - t) * a->get_x() + t * b->get_x();
        const double my = (1.0 - t) * a->get_y() + t * b->get_y();
        const double mz = (1.0 - t) * a->get_z() + t * b->get_z();

        struct FaceSplitPlan {
            facePtr face_ref;
            vertexPtr u;
            vertexPtr v;
            vertexPtr w;
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
            return nullptr;
        }

        for (auto &plan: plans) {
            const auto verts = get_oriented_face_vertices(plan.face_ref);
            bool found = false;
            for (int i = 0; i < 3; ++i) {
                const auto u = verts[i];
                const auto v = verts[(i + 1) % 3];
                const auto w = verts[(i + 2) % 3];
                if (!u || !v || !w) {
                    return nullptr;
                }
                const bool matches_ab = u->get_handle() == a->get_handle() && v->get_handle() == b->get_handle();
                const bool matches_ba = u->get_handle() == b->get_handle() && v->get_handle() == a->get_handle();
                if (matches_ab || matches_ba) {
                    plan.u = u;
                    plan.v = v;
                    plan.w = w;
                    found = true;
                    break;
                }
            }
            if (!found || !plan.u || !plan.v || !plan.w) {
                return nullptr;
            }
        }

        for (const auto &plan: plans) {
            if (!delete_face(plan.face_ref)) {
                return nullptr;
            }
        }
        if (!delete_edge(e)) {
            return nullptr;
        }

        const auto mid = add_vertex(mx, my, mz);
        if (!mid) {
            return nullptr;
        }

        for (const auto &plan: plans) {
            if (!add_face(plan.u, mid, plan.w)) {
                return nullptr;
            }
            if (!add_face(mid, plan.v, plan.w)) {
                return nullptr;
            }
        }

        return mid;
    }

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

    bool triMesh::collapse_edge(const edgePtr &e, const vertexPtr &target) {
        if (!can_collapse(e, target)) {
            return false;
        }

        const auto v0 = e->get_vertex_one();
        const auto v1 = e->get_vertex_two();
        const auto target_h = target->get_handle();
        const auto source_h = (target_h == v0->get_handle()) ? v1->get_handle() : v0->get_handle();

        std::vector<std::array<unsigned, 3> > rebuilt_faces;
        rebuilt_faces.reserve(faces_.size());

        for (const auto &f: faces_) {
            const auto verts = get_oriented_face_vertices(f);
            const bool has_target = face_contains_handle(verts, target_h);
            const bool has_source = face_contains_handle(verts, source_h);
            if (has_target && has_source) {
                continue;
            }

            auto h = to_handle_triplet(verts);
            for (auto &x: h) {
                if (x == source_h) {
                    x = target_h;
                }
            }
            if (h[0] == h[1] || h[1] == h[2] || h[2] == h[0]) {
                return false;
            }
            rebuilt_faces.push_back(h);
        }

        std::vector<vertexPtr> kept_vertices;
        kept_vertices.reserve(vertices_.size());
        for (const auto &v: vertices_) {
            if (!v || v->get_handle() == source_h) {
                continue;
            }
            kept_vertices.push_back(v);
        }

        std::sort(kept_vertices.begin(), kept_vertices.end(),
                  [](const vertexPtr &a, const vertexPtr &b) {
                      return a->get_handle() < b->get_handle();
                  });

        std::unordered_map<unsigned, vertexPtr> remap;
        remap.reserve(kept_vertices.size());

        clear_data();

        for (const auto &v: kept_vertices) {
            remap[v->get_handle()] = add_vertex(v->get_x(), v->get_y(), v->get_z());
        }

        for (const auto &tri: rebuilt_faces) {
            const auto it0 = remap.find(tri[0]);
            const auto it1 = remap.find(tri[1]);
            const auto it2 = remap.find(tri[2]);
            if (it0 == remap.end() || it1 == remap.end() || it2 == remap.end()) {
                return false;
            }
            if (!add_face(it0->second, it1->second, it2->second)) {
                return false;
            }
        }

        return true;
    }

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

    bool triMesh::flip_edge(const edgePtr &e) {
        if (!can_flip(e)) {
            return false;
        }

        const auto a = e->get_vertex_one();
        const auto b = e->get_vertex_two();
        const auto he0 = e->get_one_half_edge();
        const auto opp = he0->get_opposing_half_edge();
        const auto f0 = he0->get_parent_face();
        const auto f1 = opp->get_parent_face();

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

        std::vector<std::array<unsigned, 3> > rebuilt_faces;
        rebuilt_faces.reserve(faces_.size());
        for (const auto &f: faces_) {
            if (!f) {
                return false;
            }
            if (f->get_handle() == f0->get_handle() || f->get_handle() == f1->get_handle()) {
                continue;
            }
            const auto verts = get_oriented_face_vertices(f);
            if (!verts[0] || !verts[1] || !verts[2]) {
                return false;
            }
            rebuilt_faces.push_back(to_handle_triplet(verts));
        }

        rebuilt_faces.push_back({c->get_handle(), d->get_handle(), b->get_handle()});
        rebuilt_faces.push_back({d->get_handle(), c->get_handle(), a->get_handle()});

        std::vector<vertexPtr> kept_vertices;
        kept_vertices.reserve(vertices_.size());
        for (const auto &v: vertices_) {
            if (!v) {
                continue;
            }
            kept_vertices.push_back(v);
        }
        std::sort(kept_vertices.begin(), kept_vertices.end(),
                  [](const vertexPtr &lhs, const vertexPtr &rhs) {
                      return lhs->get_handle() < rhs->get_handle();
                  });

        std::unordered_map<unsigned, vertexPtr> remap;
        remap.reserve(kept_vertices.size());

        clear_data();

        for (const auto &v: kept_vertices) {
            remap[v->get_handle()] = add_vertex(v->get_x(), v->get_y(), v->get_z());
        }

        for (const auto &tri: rebuilt_faces) {
            const auto it0 = remap.find(tri[0]);
            const auto it1 = remap.find(tri[1]);
            const auto it2 = remap.find(tri[2]);
            if (it0 == remap.end() || it1 == remap.end() || it2 == remap.end()) {
                return false;
            }
            if (!add_face(it0->second, it1->second, it2->second)) {
                return false;
            }
        }

        return true;
    }

    // --- delete_face --------------------------------------------------
    bool triMesh::delete_face(const facePtr &f) {
        // 1) Ensure it’s in our face list
        const auto fit = std::find(faces_.begin(), faces_.end(), f);
        if (fit == faces_.end()) {
            return false;
        }

        // 2) Unhook its three half‑edges
        if (const halfEdgePtr startHE = f->get_one_half_edge()) {
            halfEdgePtr he = startHE;
            // walk exactly 3 steps
            for (int i = 0; i < 3 && he; ++i) {
                const halfEdgePtr nextHE = get_next_half_edge(he, f);

                // sever opposing link
                if (const auto opp = he->get_opposing_half_edge()) {
                    opp->set_opposing_half_edge(nullptr);
                }


                // erase from lookup
                HalfEdgeKey key{
                    he->get_vertex_one()->get_handle(),
                    he->get_vertex_two()->get_handle()
                };
                half_edge_lookup_.erase(key);

                // erase from storage
                handle_to_half_edge_.erase(he->get_handle());
                half_edges_.erase(
                    std::remove(half_edges_.begin(), half_edges_.end(), he),
                    half_edges_.end()
                );

                he = nextHE;
            }
        }

        // 3) Remove from face lookup map
        auto [a,b,c] = f->get_vertices();
        const FaceKey fk = make_face_key(a->get_handle(), b->get_handle(), c->get_handle());
        face_lookup_.erase(fk);

        // 4) Finally erase the face itself
        handle_to_face_.erase(f->get_handle());
        faces_.erase(fit);
        topology_dirty_ = true;

        return true;
    }

    // --- delete_edge --------------------------------------------------
    bool triMesh::delete_edge(const edgePtr &e) {
        const auto eit = std::find(edges_.begin(), edges_.end(), e);
        if (eit == edges_.end()) {
            return false;
        }

        // 1) Grab its two half‑edges
        const halfEdgePtr he0 = e->get_one_half_edge();
        std::vector<halfEdgePtr> hes;
        if (he0) {
            hes.push_back(he0);
        }
        if (he0 && he0->get_opposing_half_edge()) {
            hes.push_back(he0->get_opposing_half_edge());
        }

        // 2) Unhook each
        for (auto &he: hes) {
            // sever parent_edge link
            he->set_parent_edge(nullptr);
            // sever opposing
            if (const auto opp = he->get_opposing_half_edge()) {
                opp->set_opposing_half_edge(nullptr);
            }

            // erase from lookup
            HalfEdgeKey key{
                he->get_vertex_one()->get_handle(),
                he->get_vertex_two()->get_handle()
            };
            half_edge_lookup_.erase(key);

            // erase from storage
            handle_to_half_edge_.erase(he->get_handle());
            half_edges_.erase(
                std::remove(half_edges_.begin(), half_edges_.end(), he),
                half_edges_.end()
            );
        }

        // 3) Remove from edge lookup map
        {
            const auto v1 = e->get_vertex_one()->get_handle();
            const auto v2 = e->get_vertex_two()->get_handle();
            const EdgeKey ek = make_edge_key(v1, v2);
            edge_lookup_.erase(ek);
        }

        // 4) Finally erase the edge itself
        handle_to_edge_.erase(e->get_handle());
        edges_.erase(eit);
        topology_dirty_ = true;

        return true;
    }

    // --- delete_vertex ------------------------------------------------
    bool triMesh::delete_vertex(const vertexPtr &v) {
        const auto vit = std::find(vertices_.begin(), vertices_.end(), v);
        if (vit == vertices_.end()) {
            return false;
        }

        // 1) Collect all incident faces
        std::unordered_set<facePtr> facesToDel;
        for (const auto &he: v->get_outgoing_half_edges()) {
            if (auto f = he->get_parent_face()) {
                facesToDel.insert(f);
            }
        }
        for (const auto &he: v->get_incoming_half_edges()) {
            if (auto f = he->get_parent_face()) {
                facesToDel.insert(f);
            }
        }


        // 2) Delete them
        for (auto &f: facesToDel) {
            delete_face(f);
        }

        // 3) Collect any remaining edges touching v
        std::vector<edgePtr> edgesToDel;
        for (auto &e: edges_) {
            if (e->get_vertex_one() == v || e->get_vertex_two() == v) {
                edgesToDel.push_back(e);
            }
        }

        // 4) Delete those edges
        for (auto &e: edgesToDel) {
            delete_edge(e);
        }

        // 5) Finally erase the vertex itself
        handle_to_vertex_.erase(v->get_handle());
        vertices_.erase(vit);
        topology_dirty_ = true;

        return true;
    }

    int triMesh::remove_unreferenced_vertices() {
        // 1) Gather all vertices that have no incident half‑edges
        std::vector<vertexPtr> toRemove;
        toRemove.reserve(vertices_.size());
        for (auto& v : vertices_) {
            if (v->get_incoming_half_edges().empty()
             && v->get_outgoing_half_edges().empty()) {
                toRemove.push_back(v);
             }
        }

        // 2) Delete each one
        for (auto& v : toRemove) {
            // our delete_vertex will also clean up maps & vectors
            delete_vertex(v);
        }

        // 3) Return how many we removed
        return toRemove.size();
    }

    void triMesh::complete_mesh() {
        if (faces_.empty()) {
            if (const auto nRemoved = remove_unreferenced_vertices(); nRemoved > 0) {
                std::cout << "removed " << nRemoved << " unreferenced vertices" << std::endl;
            }
            topology_dirty_ = false;
            return;
        }

        // Remove unreferenced vertices
        if (const auto nRemoved = remove_unreferenced_vertices(); nRemoved > 0) {
            std::cout << "removed " << nRemoved << " unreferenced vertices" << std::endl;
        }

        // mark half-edge boundaries
        for (const auto &he: half_edges_) {
            if (const auto currentOppositeHE = he->get_opposing_half_edge(); !currentOppositeHE) {
                he->set_boundary(true);
            } else {
                he->set_boundary(false);
            }
        }

        // mark edge boundaries
        for (const auto &e: edges_) {
            if (const auto one = e->get_one_half_edge()) {
                e->set_boundary(one->is_boundary());
            }
        }

        topology_dirty_ = false;
    }


    // trivial handle‐->object
    vertexPtr triMesh::get_vertex(unsigned h) const {
        if (const auto it = handle_to_vertex_.find(h); it != handle_to_vertex_.end()) return it->second;
        return nullptr;
    }

    halfEdgePtr triMesh::get_half_edge(unsigned h) const {
        if (const auto it = handle_to_half_edge_.find(h); it != handle_to_half_edge_.end()) return it->second;
        return nullptr;
    }

    edgePtr triMesh::get_edge(unsigned h) const {
        if (const auto it = handle_to_edge_.find(h); it != handle_to_edge_.end()) return it->second;
        return nullptr;
    }

    facePtr triMesh::get_face(unsigned h) const {
        if (const auto it = handle_to_face_.find(h); it != handle_to_face_.end()) return it->second;
        return nullptr;
    }
} // namespace HalfMesh
