#include <HGraf/mesh.hpp>

#include <iostream>

namespace hg {

    /*
     * HalfEdgeMesh member functions
     */
    bool HalfEdgeMesh::validateHalfEdgeMesh() const {
        bool ok = true;

        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            bool ok0 = this->half_edges[i]->next == nullptr || this->half_edges[i]->next->next == nullptr || this->half_edges[i]->next->next->next == this->half_edges[i];
            bool ok01 = this->half_edges[i]->next == nullptr;
            bool ok02 = this->half_edges[i]->next != nullptr && this->half_edges[i]->next->next == nullptr;
            bool ok1 = this->half_edges[i]->opposite == nullptr || this->half_edges[i]->opposite->opposite == this->half_edges[i];
            bool ok2 = this->half_edges[i]->next->start_index == this->half_edges[i]->end_index;
            bool ok3 = this->half_edges[i]->opposite->start_index == this->half_edges[i]->end_index;
            bool ok4 = this->half_edges[i]->opposite->end_index == this->half_edges[i]->start_index;

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
            }

            if (!ok4) {
                std::cout << "Not ok4 on half edge " << i << std::endl;
                std::cout << "Between vertices " << this->half_edges[i]->start_index << " and " << this->half_edges[i]->end_index << std::endl;
            }


            ok = ok && ok0;
            ok = ok && ok1;
        }

        return ok;
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

        this->half_edges.push_back(rcross_edge_bottom);
        this->half_edges.push_back(rcross_edge_top);
        this->half_edges.push_back(lcross_edge_bottom);
        this->half_edges.push_back(lcross_edge_top);
        this->half_edges.push_back(top_right);
        this->half_edges.push_back(top_left);

        bool valid = this->validateHalfEdgeMesh();

        if (valid == false) {
            std::cerr << "[HGraf::HalfEdgeMesh::splitEdge] Got invalid mesh after splitting edge (or before?)" << std::endl;
            exit(-1);
        }
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
};
