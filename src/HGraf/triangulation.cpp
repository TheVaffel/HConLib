#include <HGraf/triangulation.hpp>
#include <HGraf/primitives_impl.hpp>

#include <list>
#include <iostream>
#include <map>

namespace hg {

    float rightMostCircleEdge(const falg::Vec2& p0,
                              const falg::Vec2& p1,
                              const falg::Vec2& p2) {
        falg::Vec2 v0 = p1 - p0;
        falg::Vec2 v1 = p2 - p0;
        float t = falg::dot(v1 - v0, -v1) / (2 * (v0.x() * v1.y() - v0.y() * v1.x()));

        falg::Vec2 xv0(v0.y(), -v0.x());

        falg::Vec2 middle = p0 + v0 / 2 + xv0 * t;

        float r = (middle - p0).norm();

        return middle.x() + r;

    }


    Circle getCircumCircle(const falg::Vec2& p0,
                           const falg::Vec2& p1,
                           const falg::Vec2& p2) {

        falg::Vec2 v0 = p1 - p0;
        falg::Vec2 v1 = p2 - p0;

        float cross = v0.x() * v1.y() - v0.y() * v1.x();

        float t = falg::dot(v1 - v0, -v1) / (2 * cross);

        falg::Vec2 xv0(v0.y(), -v0.x());

        falg::Vec2 middle = p0 + v0 / 2 + xv0 * t;

        float rsq = (middle - p0).sqNorm();

        return Circle(middle, sqrtf(rsq));
    }



    bool operator<(const TriInd& tr0, const TriInd& tr1) {
        return tr0.inds[0] == tr1.inds[0] ?
            (tr0.inds[1] == tr1.inds[1] ?
             (tr0.inds[2] < tr1.inds[2]) :
             tr0.inds[1] < tr1.inds[1]) :
            tr0.inds[0] < tr1.inds[0];;
    }

    bool triangleContains(const std::vector<falg::Vec2>& points,
                          const TriInd& tr0,
                          const falg::Vec2& p) {
        for(int l = 0; l < 3; l++) {
            int nl = (l + 1) % 3;

            falg::Vec2 pt = points[tr0.inds[l]];
            falg::Vec2 d = points[tr0.inds[nl]] - pt;
            falg::Vec2 dx(- d.y(), d.x());

            falg::Vec2 dp = p - pt;
            float cross = d.x() * dp.y() - dp.x() * d.y();

            if(cross < 0) {
                return false;
            }
        }

        return true;
    }


    // Implementation of the Bowyer-Watson algorithm
    DelaunayTriangles delaunay_triangulation(const std::vector<falg::Vec2>& in_points,
                                             int boundary_x, int boundary_y,
                                             int boundary_w, int boundary_h) {

        struct EdgeInd {
            uint32_t inds[2];

            bool operator<(const EdgeInd& e1) const {
                return this->inds[0] == e1.inds[0] ?
                          this->inds[1] < e1.inds[1] :
                    this->inds[0] < e1.inds[0];
            }
        };

        DelaunayTriangles del_tri;
        std::list<TriInd> triangulation;
        std::list<Circle> circumcircles;

        int num_discarded = 0;
        for(uint32_t i = 0; i < in_points.size(); i++) {
            bool ok = true;
            for(uint32_t j = i + 1; j < in_points.size(); j++) {
                if((in_points[i] - in_points[j]).sqNorm() < boundary_w * boundary_h * 1e-4) {
                    ok = false;
                    break;
                }
            }

            if(ok) {
                del_tri.points.push_back(in_points[i]);
            } else {
                num_discarded++;
            }
        }

        std::cerr << "[hg::delaunay_triangulation] Discarded "
                  << num_discarded << " points to avoid degeneracy" << std::endl;

        std::vector<falg::Vec2>& points = del_tri.points;

        boundary_x -= 5;
        boundary_y -= 5;
        boundary_w += 10;
        boundary_h += 10;


        falg::Vec2 p0 = falg::Vec2(boundary_x, boundary_y),
            p1 = falg::Vec2(boundary_x + boundary_w, boundary_y),
            p2 = falg::Vec2(boundary_x, boundary_y + boundary_h),
            p3 = falg::Vec2(boundary_x + boundary_w, boundary_y + boundary_h);


        TriInd tri0 = { (uint32_t)points.size() + 0,
                        (uint32_t)points.size() + 1,
                        (uint32_t)points.size() + 2 };
        TriInd tri1 = { (uint32_t)points.size() + 2,
                        (uint32_t)points.size() + 1,
                        (uint32_t)points.size() + 3 };

        points.push_back(p0);
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);

        triangulation.push_back(tri0);
        triangulation.push_back(tri1);
        circumcircles.push_back(getCircumCircle(points[tri0.inds[0]], points[tri0.inds[1]], points[tri0.inds[2]]));
        circumcircles.push_back(getCircumCircle(points[tri1.inds[0]], points[tri1.inds[1]], points[tri1.inds[2]]));

        // Add new points, one at a time (exclude 4 points added above)
        for(unsigned int i = 0; i < points.size() - 4; i++) {

            // For the new point, find all existing triangles whose circumcircle contains the point
            std::vector<std::pair<std::list<TriInd>::iterator,
                                  std::list<Circle>::iterator>> bad_triangles;
            std::list<Circle>::iterator cit = circumcircles.begin();
            for(std::list<TriInd>::iterator it = triangulation.begin(); it != triangulation.end(); it++) {

                if((*cit).contains(points[i])) {
                    bad_triangles.push_back(std::make_pair(it, cit));
                }

                cit++;
            }

            // For all such bad triangles, count occurences of each edge
            std::map<EdgeInd, int> edge_count;
            for(unsigned int k = 0; k < bad_triangles.size(); k++) {
                for(unsigned int l = 0; l < 3; l++) {
                    int b0 = l;
                    int b1 = (l + 1) % 3;
                    uint32_t mintr = std::min((*bad_triangles[k].first).inds[b0],
                                              (*bad_triangles[k].first).inds[b1]);
                    uint32_t maxtr = std::max((*bad_triangles[k].first).inds[b0],
                                              (*bad_triangles[k].first).inds[b1]);
                    if(edge_count.find({mintr, maxtr}) != edge_count.end()) {
                        edge_count[{mintr, maxtr}]++;
                    } else {
                        edge_count[{mintr, maxtr}] = 1;
                    }
                }
            }

            // For each edge that are only counted once (not shared among bad triangles),
            // add that edge to the outline of the polygon of the bad triangles
            std::vector<EdgeInd> polygon;
            for(unsigned int k = 0; k < bad_triangles.size(); k++) {
                for(unsigned int l = 0; l < 3; l++) {
                    int b0 = l;
                    int b1 = (l + 1) % 3;

                    uint32_t mintr = std::min((*bad_triangles[k].first).inds[b0],
                                              (*bad_triangles[k].first).inds[b1]);
                    uint32_t maxtr = std::max((*bad_triangles[k].first).inds[b0],
                                              (*bad_triangles[k].first).inds[b1]);

                    if(edge_count[{mintr, maxtr}] == 1) {
                        polygon.push_back({mintr, maxtr});
                    }
                }

                // Erase bad triangle from triangulation list (together with its circumcircle)
                triangulation.erase(bad_triangles[k].first);
                circumcircles.erase(bad_triangles[k].second);
            }

            // For each edge in the outline polygon, add a new triangle using the new point as the
            // third vertex
            for(std::map<EdgeInd, int>::iterator it = edge_count.begin(); it != edge_count.end(); it++) {
                int num = (*it).second;
                if(num > 1) {
                    continue;
                }

                TriInd tri;
                tri.inds[0] = (*it).first.inds[0];
                tri.inds[1] = (*it).first.inds[1];
                tri.inds[2] = i;

                triangulation.push_back(tri);
                circumcircles.push_back(getCircumCircle(points[tri.inds[0]], points[tri.inds[1]], points[tri.inds[2]]));

            }
        }

        // Store triangles in result vector
        for(std::list<TriInd>::iterator it = triangulation.begin(); it != triangulation.end(); it++) {
            TriInd pr = (*it);

            del_tri.indices.push_back(pr);
        }

        // Switch winding direction for triangles whos direction is wrong
        for(uint32_t i = 0; i < del_tri.indices.size(); i++) {
            falg::Vec2& p0 = del_tri.points[del_tri.indices[i].inds[0]];
            falg::Vec2& p1 = del_tri.points[del_tri.indices[i].inds[1]];
            falg::Vec2& p2 = del_tri.points[del_tri.indices[i].inds[2]];

            falg::Vec2 d1 = p1 - p0;
            falg::Vec2 d2 = p2 - p0;

            if(d1.x() * d2.y() - d1.y() * d2.x() < 0) {
                // Wrong winding direction, swap two indices
                std::swap(del_tri.indices[i].inds[0],
                          del_tri.indices[i].inds[1]);
            }
        }

        return del_tri;
    }
};
