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
        const auto key = make_face_key(v1->get_handle(),
                                       v2->get_handle(),
                                       v3->get_handle());
        if (const auto it = face_lookup_.find(key); it != face_lookup_.end()) {
            return handle_to_face_[it->second];
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

        // 3) pull out the three half‐edges that bound this face:
        //    Edge::get_one_half_edge() is guaranteed to be the he in *this* face
        const auto he1 = e1->get_one_half_edge();
        const auto he2 = e2->get_one_half_edge();
        const auto he3 = e3->get_one_half_edge();

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
        if (faces_.empty()) return;

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
