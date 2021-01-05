#pragma once

#include <FlatAlg.hpp>

#include <vector>

namespace hg {
    struct TriInd {
        uint32_t inds[3];
    };
    bool operator<(const TriInd& tr0, const TriInd& tr1);

    bool triangleContains(const std::vector<falg::Vec2>& points,
                          const TriInd& tri0,
                          const falg::Vec2& p);

    struct DelaunayTriangles {
        std::vector<falg::Vec2> points;
        std::vector<TriInd> indices;
    };

    DelaunayTriangles delaunay_triangulation(const std::vector<falg::Vec2>& in_points,
                                             int bx, int by, int bw, int bh);
};
