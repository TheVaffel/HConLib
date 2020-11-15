#include <HGraf.hpp>
#include <Winval.hpp>

#include <random>
#include <vector>
// #include <algorithm>

#include <iostream>

int main() {
    const int width = 800;
    const int height = 800;
    const int num_points = 30;

    
    Winval win(width, height);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> dist_x(0.f, (float)width);
    std::uniform_real_distribution<float> dist_y(0.f, (float)height);

    std::vector<falg::Vec2> points;

    for(int i = 0; i < num_points; i++) {
        falg::Vec2 p(dist_x(generator), dist_y(generator));

        points.push_back(p);
    }

    hg::Canvas cv(width, height);
    for(int i = 0; i < num_points; i++) {
        cv.setPixel((int)points[i].x(), (int)points[i].y(), (255 << 24) | (255 << 16) | (255 << 8));
    }

    hg::DelaunayTriangles tris = hg::delaunay_triangulation(points, 0, 0, width, height);

    for(unsigned int i = 0; i < tris.indices.size(); i++) {
        for(int l = 0; l < 3; l++) {
            int nl = (l + 1) % 3;
            hg::drawLineSafe(cv, (int)tris.points[tris.indices[i].inds[l]].x(),
                             (int)tris.points[tris.indices[i].inds[l]].y(),
                             (int)tris.points[tris.indices[i].inds[nl]].x(),
                             (int)tris.points[tris.indices[i].inds[nl]].y(),
                             (255 << 24) | (255 << 8));
        }
        
    }

    win.drawBuffer(cv.getData(), width, height);

    win.waitForKey();
}
