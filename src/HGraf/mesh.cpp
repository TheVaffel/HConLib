#include <HGraf/mesh.hpp>

#include <iostream>
#include <set>

namespace hg {

    /*
     * HalfEdgeMesh member functions
     */
    bool HalfEdgeMesh::validateHalfEdgeMesh() const {
        bool ok = true;

        std::set<std::pair<int, int> > found_indices;

        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            bool ok0 = this->half_edges[i]->next == nullptr || this->half_edges[i]->next->next == nullptr || this->half_edges[i]->next->next->next == this->half_edges[i];
            bool ok01 = this->half_edges[i]->next == nullptr;
            bool ok02 = this->half_edges[i]->next != nullptr && this->half_edges[i]->next->next == nullptr;
            bool ok1 = this->half_edges[i]->opposite == nullptr || this->half_edges[i]->opposite->opposite == this->half_edges[i];
            bool ok2 = this->half_edges[i]->next->start_index == this->half_edges[i]->end_index;
            bool ok3 = this->half_edges[i]->opposite->start_index == this->half_edges[i]->end_index;
            bool ok4 = this->half_edges[i]->opposite->end_index == this->half_edges[i]->start_index;
            bool ok5 = this->half_edges[i]->index_in_half_edge_mesh == i;

            std::pair<int, int> pp = std::make_pair(this->half_edges[i]->start_index, this->half_edges[i]->end_index);
            bool ok6 = found_indices.find(pp) == found_indices.end();

            found_indices.insert(pp);

            if (!ok0) {
                std::cout << "Not ok0 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
                std::cout << "Next null = " << ok01 << ", nextnext null = " << ok02 << std::endl;
            }

            if (!ok1) {
                std::cout << "Not ok1 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
            }

            if (!ok2) {
                std::cout << "Not ok2 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
            }

            if (!ok3) {
                std::cout << "Not ok3 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
                std::cout << "Had registered index " << this->half_edges[i]->index_in_half_edge_mesh << std::endl;
                std::cout << "Opposite edge had indices " << this->half_edges[i]->opposite->start_index << " and " << this->half_edges[i]->opposite->end_index << std::endl;
            }

            if (!ok4) {
                std::cout << "Not ok4 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
            }

            if (!ok5) {
                std::cout << "Not ok5 on half edge " << i << std::endl;
                std::cout << "Edge did not have correct index in half_edge list, had index " << this->half_edges[i]->index_in_half_edge_mesh << std::endl;
            }

            if (!ok6) {
                std::cout << "Not ok6 on half edge " << i << std::endl;
                std::cout << "Edge between " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << " was already inserted " << std::endl;
            }

            bool dook = false;
            int ti = 0;
            HalfEdge* ce = this->half_edges[i];
            while (ti++ < 1000) {
                ce = ce->next->opposite;
                if (ce == this->half_edges[i]) {
                    dook = true;
                    break;
                }
            }

            if (! dook) {
                std::cout << "Travel around vertex " << this->half_edges[i]->end_index << " starting at edge " << i << " failed " << std::endl;
            }


            ok = ok && ok0 && ok1 && ok2 && ok3 && ok4 && ok5 && dook && ok6;
        }

        return ok;
    }

    void HalfEdgeMesh::add_edge_to_list(HalfEdge* edge) {
        edge->index_in_half_edge_mesh = this->half_edges.size();
        this->half_edges.push_back(edge);
    }

    void HalfEdgeMesh::splitEdge(HalfEdge *edge, int new_vertex_index) {
        int v1 = edge->end_index;

        int rv = edge->next->end_index;
        int lv = edge->opposite->next->end_index;

        // Here, the original edge is the right half of a vertical edge going upward.
        // Right and left, top and bottom are defined accordingly

        HalfEdge *rcross_edge_bottom = new HalfEdge,
            *rcross_edge_top = new HalfEdge,
            *lcross_edge_bottom = new HalfEdge,
            *lcross_edge_top = new HalfEdge,
            *top_right = new HalfEdge,
            *top_left = new HalfEdge;



        rcross_edge_bottom->start_index = new_vertex_index; rcross_edge_bottom->end_index = rv;
        rcross_edge_bottom->next = edge->next->next; rcross_edge_bottom->opposite = rcross_edge_top;

        rcross_edge_top->start_index = rv; rcross_edge_top->end_index = new_vertex_index;
        rcross_edge_top->next = top_right; rcross_edge_top->opposite = rcross_edge_bottom;

        lcross_edge_bottom->start_index = lv; lcross_edge_bottom->end_index = new_vertex_index;
        lcross_edge_bottom->next = edge->opposite; lcross_edge_bottom->opposite = lcross_edge_top;

        lcross_edge_top->start_index = new_vertex_index; lcross_edge_top->end_index = lv;
        lcross_edge_top->next = edge->opposite->next->next; lcross_edge_top->opposite = lcross_edge_bottom;

        top_right->start_index = new_vertex_index; top_right->end_index = v1;
        top_right->next = edge->next; top_right->opposite = top_left;

        top_left->start_index = v1; top_left->end_index = new_vertex_index;
        top_left->next = lcross_edge_top; top_left->opposite = top_right;

        edge->opposite->start_index = new_vertex_index; // Opposite from start already ends at bottom
        // Opposite and next are already as they should

        edge->end_index = new_vertex_index; // Start is already bottom
        edge->next = rcross_edge_bottom; // Opposite is already right

        // Correct bounding edges (that were untouched above)
        edge->opposite->next->next = lcross_edge_bottom;
        lcross_edge_top->next->next = top_left;
        top_right->next->next = rcross_edge_top;
        // rcross_edge_bottom->next-next is already the original edge

        this->add_edge_to_list(rcross_edge_bottom);
        this->add_edge_to_list(rcross_edge_top);
        this->add_edge_to_list(lcross_edge_bottom);
        this->add_edge_to_list(lcross_edge_top);
        this->add_edge_to_list(top_right);
        this->add_edge_to_list(top_left);

        /* bool valid = this->validateHalfEdgeMesh();

        if (valid == false) {
            std::cerr << "[HGraf::HalfEdgeMesh::splitEdge] Got invalid mesh after splitting edge (or before?)" << std::endl;
            exit(-1);
            } */
    }

    void HalfEdgeMesh::remove_edge_from_list(HalfEdge* edge) {
        int i = edge->index_in_half_edge_mesh;
        this->half_edges[i] = this->half_edges[this->half_edges.size() - 1];
        this->half_edges[i]->index_in_half_edge_mesh = i;
        this->half_edges.pop_back();
    }

    // Returns deleted vertex index
    int HalfEdgeMesh::mergeEdge(HalfEdge* edge) {
        int vert_to_keep = edge->start_index;
        int vert_to_delete = edge->end_index;

        HalfEdge* top_right_keep = edge->next->opposite;
        HalfEdge* bottom_right_keep = edge->next->next->opposite;
        HalfEdge* bottom_left_keep = edge->opposite->next->opposite;
        HalfEdge* top_left_keep = edge->opposite->next->next->opposite;

        int dudu = 0;
        HalfEdge* cur = top_right_keep->next;
        while (cur != top_left_keep) {
            // Check whether any of the affected triangles already refers to the start index (then this operation is aborted)
            HalfEdge* ccur = cur->next->opposite;
            while (ccur != cur) {
                if (ccur->start_index == vert_to_keep) {
                    return -1;
                }

                ccur = ccur->next->opposite;
            }

            dudu++;
            cur = cur->opposite->next;
        }

        this->remove_edge_from_list(top_right_keep->opposite);
        this->remove_edge_from_list(bottom_right_keep->opposite);
        this->remove_edge_from_list(bottom_left_keep->opposite);
        this->remove_edge_from_list(top_left_keep->opposite);
        this->remove_edge_from_list(edge->opposite);
        this->remove_edge_from_list(edge);

        top_right_keep->opposite = bottom_right_keep;
        bottom_right_keep->opposite = top_right_keep;

        top_left_keep->opposite = bottom_left_keep;
        bottom_left_keep->opposite = top_left_keep;

        cur = bottom_right_keep;
        while (cur != top_left_keep) {


            cur->opposite->end_index = vert_to_keep;
            cur->opposite->next->start_index = vert_to_keep;

            cur = cur->opposite->next;
        }

        /* bool valid = this->validateHalfEdgeMesh();

        if (valid == false) {
            std::cerr << "[HGraf::HalfEdgeMesh::mergeEdge] Got invalid mesh after merging edge (or before?)" << std::endl;
            exit(-1);
            } */

        return vert_to_delete;
    }


    void HalfEdgeMesh::splitTriangle(HalfEdge* one_edge, int new_vertex_index) {
        HalfEdge *h1 = new HalfEdge,
            *h1o = new HalfEdge,
            *h2 = new HalfEdge,
            *h2o = new HalfEdge,
            *h3 = new HalfEdge,
            *h3o = new HalfEdge;

        HalfEdge *e1 = one_edge;
        HalfEdge *e2 = e1->next;
        HalfEdge *e3 = e2->next;

        this->add_edge_to_list(h1);
        this->add_edge_to_list(h1o);
        this->add_edge_to_list(h2);
        this->add_edge_to_list(h2o);
        this->add_edge_to_list(h3);
        this->add_edge_to_list(h3o);

        e1->next = h1;
        h1->next = h3o;
        h3o->next = e1;

        e2->next = h2;
        h2->next = h1o;
        h1o->next = e2;

        e3->next = h3;
        h3->next = h2o;
        h2o->next = e3;

        h1->opposite = h1o;
        h1o->opposite = h1;

        h2->opposite = h2o;
        h2o->opposite = h2;

        h3->opposite = h3o;
        h3o->opposite = h3;

        h1->start_index = e1->end_index;
        h1->end_index = new_vertex_index;
        h1o->start_index = new_vertex_index;
        h1o->end_index = h1->start_index;

        h2->start_index = e2->end_index;
        h2->end_index = new_vertex_index;
        h2o->start_index = new_vertex_index;
        h2o->end_index = h2->start_index;

        h3->start_index = e3->end_index;
        h3->end_index = new_vertex_index;
        h3o->start_index = new_vertex_index;
        h3o->end_index = h3->start_index;

        bool valid = this->validateHalfEdgeMesh();

        if (valid == false) {
            std::cerr << "[HGraf::HalfEdgeMesh::splitTriangle] Got invalid mesh after merging edge (or before?)" << std::endl;
            exit(-1);
        }
    }


    void HalfEdgeMesh::constructIndices(std::vector<uint32_t>& indices) const {
        indices.clear();

        std::set<std::pair<int, int>> taken_edges;

        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            HalfEdge* p = this->half_edges[i];
            if (taken_edges.find(std::make_pair(p->start_index, p->end_index)) != taken_edges.end()) {
                // Edge already added, continue
                continue;
            }

            for (int j = 0; j < 3; j++) {
                indices.push_back(p->start_index);
                taken_edges.insert(std::make_pair(p->start_index, p->end_index));
                p = p->next;
            }
        }
    }

    void HalfEdgeMesh::reconstructMesh(NormalMesh& mesh) {
        std::vector<int> index_map(mesh.positions.size(), -1);

        std::vector<falg::Vec3> new_positions;
        std::vector<falg::Vec3> new_normals;

        int curr_ind = 0;
        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            HalfEdge* edge = this->half_edges[i];
            int v0 = edge->start_index;
            int v1 = edge->end_index;
            if (index_map[v0] < 0) {
                index_map[v0] = curr_ind++;
                new_positions.push_back(mesh.positions[v0]);
                new_normals.push_back(mesh.normals[v0]);
            }

            if (index_map[v1] < 0) {
                index_map[v1] = curr_ind++;
                new_positions.push_back(mesh.positions[v1]);
                new_normals.push_back(mesh.normals[v1]);
            }

            edge->start_index = index_map[v0];
            edge->end_index = index_map[v1];
        }

        mesh.positions = new_positions;
        mesh.normals = new_normals;
        this->constructIndices(mesh.indices);
    }

    const HalfEdge* const* HalfEdgeMesh::getData() const {
        return half_edges.data();
    }

    HalfEdge** HalfEdgeMesh::getData() {
        return half_edges.data();
    }

    unsigned int HalfEdgeMesh::getSize() const {
        return half_edges.size();
    }

    HalfEdgeMesh::~HalfEdgeMesh() {
        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            delete half_edges[i];
        }
    }
};
