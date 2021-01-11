#pragma once

#include "mesh.hpp"

#include <map>
#include <algorithm>

#include <iostream>

namespace hg {

    template<Mesh_C T>
    HalfEdgeMesh::HalfEdgeMesh(const T& mesh) : half_edges(mesh.indices.size()) {

        for (unsigned int i = 0; i < this->half_edges.size(); i++) {
            this->half_edges[i] = new HalfEdge;
        }

        std::map<std::pair<int, int>, int> indmap;

        for (unsigned int i = 0; i < mesh.indices.size(); i++) {
            int tbase = (i / 3) * 3;
            int ti = i % 3;
            int curr_half_edge = i;
            int next_half_edge = (ti + 1) % 3 + tbase;

            int curr_index = mesh.indices[curr_half_edge];
            int next_index = mesh.indices[next_half_edge];

            this->half_edges[curr_half_edge]->next = this->half_edges[next_half_edge];

            std::pair<int, int> curInds(std::min(curr_index, next_index),
                                        std::max(curr_index, next_index));

            this->half_edges[curr_half_edge]->start_index = curr_index;
            this->half_edges[curr_half_edge]->end_index = next_index;

            if (curr_index == next_index) {
                std::cout << "Same index! curr_half = " << curr_half_edge << ", next_half = " << next_half_edge << ", curInds = " << curInds.first << ", " << curInds.second << std::endl;
            }

            if (indmap.find(curInds) != indmap.end()) {
                this->half_edges[curr_half_edge]->opposite = this->half_edges[indmap[curInds]];
                this->half_edges[indmap[curInds]]->opposite = this->half_edges[curr_half_edge];
            } else {
                indmap[curInds] = curr_half_edge;
            }
        }

        bool valid = this->validateHalfEdgeMesh();

        if (!valid) {
            std::cout << "[HGraf::HalfEdgeMesh] Oh no, HalfEdgeMesh invalid :( :(" << std::endl;
        }
    }
};
