#include <cmath>
#include <HGraf.hpp>
#include <algorithm>

#include <iostream>
#include <list>
#include <iterator>
#include <map>

static int msb(int a) {
#ifdef WIN32
    int i = 31;
    while((a&(1<<i)) == 0 && i > 0) {
        i--;
    }
    return i;
#else //WIN32
    return 31 - __builtin_clz(a);
#endif //WIN32
}

namespace hg {
  
    using namespace falg;
    Rectangle::Rectangle() {}
  
    Rectangle::Rectangle(float nx, float ny, float nw, float nh) {
        x = nx; y = ny; w = nw; h = nh;
    }
  
    float Rectangle::getX() const {
        return x;
    }

    float Rectangle::getY() const {
        return y;
    }

    float Rectangle::getWidth() const {
        return w;
    }

    float Rectangle::getHeight() const {
        return h;
    }

    void Rectangle::setX(float nx) {
        x = nx;
    }

    void Rectangle::setY(float ny) {
        y = ny;
    }

    void Rectangle::setWidth(float nw) {
        w = nw;
    }

    void Rectangle::setHeight(float nh) {
        h = nh;
    }

    bool Rectangle::contains(const Vec2& p) const {
        return p[0] >= x && p[1] >= y && p[0] <= x + w && p[1] <= y + h;
    }

    void drawRectangle(Canvas* canvas, const Rectangle& rectangle, const Color c) {
        int ic = colorToInt(c);
    
        int ix = (int)rectangle.getX(), iy = (int)rectangle.getY();
        int iw = (int)rectangle.getWidth(), ih = (int)rectangle.getHeight();

        int fx1 = std::max(std::min(ix, canvas->getWidth() - 1), 0);
        int fx2 = std::max(std::min(ix + iw, canvas->getWidth() - 1), 0);
        int fy1 = std::max(std::min(iy, canvas->getHeight() - 1), 0);
        int fy2 = std::max(std::min(iy + ih, canvas->getHeight() - 1), 0);

        if(iy == fy1) {
            for(int i = fx1; i < fx2 ; i++) {
                canvas->setPixel(i, iy, ic);
            }
        }
    
        if (iy + ih == fy2) {
            for(int i = fx1; i < fx2 ; i++) {
                canvas->setPixel(i, iy + ih, ic);
            }
        }

        if(ix == fx1) {
            for(int i = fy1; i < fy2; i++) {
                canvas->setPixel(ix, i, ic);
            }
        }

        if(ix + iw == fx2) {
            for(int i = fy1; i < fy2; i++) {
                canvas->setPixel(ix + iw, i, ic);
            }
        }
    }

    inline int _signum(int a) {
        return (a > 0) - (a < 0);
    }
  
    inline int colorToInt(Color c) {
        return (255 << 24) | (((int)(c[0]*255))<<16) | (((int)(c[1]*255))<<8) | ((int)(c[2]*255));
    }

    //An implementation of Bresenham's algorithm
    void drawLine(unsigned char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum) {
    
        int dx = endx - startx, dy = endy - starty;
    
        if(dx == 0 && dy == 0) {
            ((int*)buffer)[w*starty + startx] = colorNum;
            return;
        }
        int psex;
        int stepx, stepy;
        int ny = starty;
        int nx = startx;
        int* iterx;
        int* itery;

        int err = 0;

        if(std::abs(dx) < std::abs(dy)) {
            psex = endy;
            iterx = &ny;
            itery = &nx;
      
            int a = dx;
            dx = dy;
            dy = a;
        }else{
            psex = endx;

            iterx = &nx;
            itery = &ny;
        }

        if(dx < 0) {
            stepx = -1;
            dx = -dx;
        }else{
            stepx = 1;
        }

        if(dy < 0) {
            dy = -dy;
            stepy = -1;
        }else{
            stepy = 1;
        }

        for(; *iterx != psex; *iterx += stepx) {
            ((int*)buffer)[w*ny + nx] = colorNum;
            err += dy;
            if(err > dx) {
                err-=dx;
                *itery += stepy;
            }
        }
    }

    inline int _cross(int x1, int y1, int x2, int y2) {
        return x1*y2 - x2*y1;
    }

    //return whether it is possible
  
    bool moveEndpointsOntoScreen(int& sx, int& sy, int& ex, int& ey, int w, int h) {
    
        int u = _signum(_cross(ex - sx, ey - sy, ex - 0, ey - 0));
        if((sx < 0 ||sy < 0 || sx >=w || sy >= h) && (ex < 0 || ey < 0 || ex >=w || ey >= h) &&
           _signum(_cross(ex - sx, ey - sy, ex - 0, ey - h)) == u &&
           _signum(_cross(ex - sx, ey - sy, ex - w, ey - 0)) == u &&
           _signum(_cross(ex - sx, ey - sy, ex - w, ey - h)) == u) {
            return false;
        }
    
        if(sx < 0) {
            sy = (int)(((float)-sx)/(ex - sx)*(ey - sy)) + sy;
            sx = 0;
        }else if(sx >=w) {
            sy = (int)(((float)w-sx - 1)/(ex-sx)*(ey - sy)) + sy;
            sx = w - 1;
        }

        if(sy < 0) {
            sx =(int)(((float)-sy)/(ey - sy)*(ex - sx)) + sx;
            sy = 0;
        }else if(sy >= h) {
            sx = (int)(((float)h - sy -1)/(ey - sy)*(ex - sx)) + sx;
            sy  = h - 1;
        }

        if(ex < 0) {
            ey = (int)(((float)-ex)/(sx - ex)*(sy - ey)) + ey;
            ex = 0;
        }else if(ex >=w) {
            ey = (int)(((float)w-ex - 1)/(sx-ex)*(sy - ey)) + ey;
            ex = w - 1;
        }

        if(ey < 0) {
            ex =(int)(((float)-ey)/(sy - ey)*(sx - ex)) + ex;
            ey = 0;
        }else if(ey >= h) {
            ex = (int)(((float)h - ey -1)/(sy - ey)*(sx - ex)) + ex;
            ey  = h - 1;
        }

        return true;
    }

  
    void drawLineSafe(unsigned char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum) {
        if(!moveEndpointsOntoScreen(startx, starty, endx, endy, w, h)) {
            return;
        }

        hg::drawLine(buffer, w, h, startx, starty, endx, endy, colorNum);
    }

    void drawLineSafe(Canvas& canvas, int startx, int starty, int endx, int endy, int colorNum) {
        drawLineSafe(canvas.getData(), canvas.getWidth(), canvas.getHeight(), startx, starty, endx, endy, colorNum);
    }


    //Similar to the one above, but returns all line points
    void getLinePoints(int x0, int y0, int x1, int y1, int poss[][2], int& numposs) {
        int dx = x1 - x0, dy = y1 - y0;
        int pssx, psex, pssy;
        int stepx, stepy;
  
        int num = 0;
        int err = 0;
        int xind = 0, yind = 1;


        if(std::abs(dx) < std::abs(dy)) {
            pssx = y0, psex = y1, pssy = x0;
            int a = dx;
            dx = dy;
            dy = a;
            xind = 1; yind = 0;
        }else{
            pssx = x0, psex = x1, pssy = y0;
        }

        if(dx < 0) {
            stepx = -1;
            dx = -dx;
        }else{
            stepx = 1;
        }

        if(dy < 0) {
            dy = -dy;
            stepy = -1;
        }else{
            stepy = 1;
        }

        int ny = pssy;

        for(int nx = pssx; nx != psex; nx += stepx) {
            poss[num][xind] = nx;
            poss[num++][yind] = ny;
            err += dy;
            if(err > dx) {
                err-=dx;
                ny += stepy;
            }
        }
        numposs = num;
    }

    LineIterator::LineIterator(int x, int y) {
        currPoint[0] = x, currPoint[1] = y;
    }

    LineIterator::LineIterator(int sx, int sy, int ex, int ey) {
        dd[0] = ex - sx; dd[1] =  ey - sy;
        endPoint[0] = ex, endPoint[1] = ey;
        currPoint[0] = sx, currPoint[1] = sy;
        curr = 0;
        if(std::abs(dd[0]) > std::abs(dd[1])) {
            ground = 0;
        }else{
            ground = 1;
        }
        notGround = ground ^ 1;
        stepGround = dd[ground]>=0?1:-1;
        stepNotGround = dd[notGround] >= 0?1:-1;
        dd[0] = std::abs(dd[0]);
        dd[1] = std::abs(dd[1]);
    }
  
    LineIterator::LineIterator(const LineIterator& it) {
        currPoint[0] = it[0];
        currPoint[1] = it[1];
    }
    int LineIterator::operator[](int i) const{
        return currPoint[i];
    }
  
    LineIterator LineIterator::operator++(int i) {
        LineIterator li(*this);
        currPoint[ground] += stepGround;
        curr+= dd[notGround];
        if(curr >= dd[ground]) {
            curr -= dd[ground];
            currPoint[notGround]+=stepNotGround;
        }
        return li;
    }
  
    LineIterator LineIterator::end() {
        return LineIterator(endPoint[0], endPoint[1]);
    }
  
    bool LineIterator::operator==(const LineIterator& li) {
        return currPoint[0] == li[0] && currPoint[1] == li[1];
    }

    bool LineIterator::isFinished() const{
        return currPoint[ground] - stepGround == endPoint[ground];
    }

    void getBoundaryIntersections(const int point[2], const float vector[2], const int boundary[2][2], int endPoints[2][2]) {

        int right = boundary[1][0] - point[0], left = boundary[0][0] - point[0],
            up = boundary[0][1] - point[1], down = boundary[1][1] - point[1];

        //Check against right quarter
        if((vector[0] > 0 && vector[1] < 0) || (vector[1] > 0 && vector[0] < 0)) {
            int temp = right;
            right = left;
            left = temp;
        }

        float a, b;

        if((a = std::abs(right*vector[1])) < (b = std::abs(down*vector[0]))) {
            endPoints[0][0] = point[0] + right;
            endPoints[0][1] = point[1] + (int)(right*vector[1]/vector[0]);
        }else if(a > b) {
            endPoints[0][0] = point[0] + (int)(down*vector[0]/vector[1]);
            endPoints[0][1] = point[1] + down;
        }else{
            endPoints[0][0] = point[0] + right;
            endPoints[0][1] = point[1] + down;
        }

        if((a = std::abs(left*vector[1])) < (b = std::abs(up*vector[0]))) {
            endPoints[1][0] = point[0] + left;
            endPoints[1][1] = point[1] + (int)(left*vector[1]/vector[0]);
        }else if(a > b) {
            endPoints[1][0] = point[0] + (int)(up*vector[0]/vector[1]);
            endPoints[1][1] = point[1] + up;
        }else{
            endPoints[1][0] = point[0] + left;
            endPoints[1][1] = point[1] + up;
        }
    }

    void cutLineToZPlane(const Vec3& p1, const Vec3& p2, float plane, Vec3& dst1, Vec3& dst2) {
        Vec3 v = p2 - p1;

        if(p1.z() > -plane) {
            dst1 = p1 - v*((p1.z() + plane)/v.z());
        }else dst1 = p1;

        if(p2.z() > -plane) {
            dst2 = p2 - v*((p2.z() + plane)/v.z());
        }else dst2 = p2;
    }


    void drawLine3D(Canvas& canvas, const CamParam& camparam, const Vec3& start, const Vec3& end, int color) {
        Vec3 nstart, nend;

        if(start.z() > -camparam.nearPlane && end.z() > -camparam.nearPlane) {
            return;
        }
    
        cutLineToZPlane(start, end, camparam.nearPlane, nstart, nend);

        int ps[2] = {(int)((-nstart.x()/nstart.z()*camparam.invtanfovhover2 + 1.0f)*camparam.screenWidth)/2,
                     (int)((nstart.y()/nstart.z()*camparam.invtanfovvover2 + 1.0f)*camparam.screenHeight)/2};
        int pe[2] = {(int)((-nend.x()/nend.z()*camparam.invtanfovhover2 + 1.0f)*camparam.screenWidth)/2,
                     (int)((nend.y()/nend.z()*camparam.invtanfovvover2 + 1.0f)*camparam.screenHeight)/2};

        int di[2] = {pe[0] - ps[0], pe[1] - ps[1]};
        if(std::abs(di[0]) > 2*canvas.getWidth() || std::abs(di[1]) > 2*canvas.getHeight()) {
            if(std::abs(ps[0]) > canvas.getWidth() || std::abs(ps[1] > canvas.getHeight())) {
                int msb_wh = std::max(msb(canvas.getWidth()), msb(canvas.getHeight()));
                int msb_ps = std::min(msb(ps[0]), msb(ps[1])); //Most significant bit of ps
                int shift = msb_ps - msb_wh - 1;
                if(shift > 0) {
                    ps[0] = (ps[0]<0?-( ((int)std::abs(ps[0] - pe[0]))>>shift) : (ps[0] + pe[0])>>shift) + pe[0];
                    ps[1] = (ps[1]<0?-( ((int)std::abs(ps[1] - pe[1]))>>shift) : (ps[1] + pe[1])>>shift) + pe[1];
                }
                int msb_pe = std::min(msb(pe[0]), msb(pe[1]));
                shift = msb_pe - msb_wh - 1;
                if(shift > 0) {
                    pe[0] = (pe[0]<0?-( ((int)std::abs(pe[0] - ps[0]))>>shift) : (pe[0] + ps[0])>>shift) + ps[0];
                    pe[1] = (pe[1]<0?-( ((int)std::abs(pe[1] - ps[1]))>>shift) : (pe[1] + ps[1])>>shift) + ps[1];
                }
            }
        }
    
        drawLineSafe(canvas, ps[0], ps[1], pe[0], pe[1], color);
    }
  
    void drawLineModel(Canvas& canvas, const CamParam& camparam, const LineModel& model, const Mat4& mat, int color) {
        Vec3* p = new Vec3[model.numPoints];
        for(int i = 0; i< model.numPoints; i++) {
            p[i] = mat*model.points[i];
        }
        for(int i = 0; i< model.numIndices; i++) {
      
            drawLine3D(canvas, camparam, p[model.indices[2*i]], p[model.indices[2*i + 1]], color);
      
        }
        delete[] p;
    }

    void clearCanvas(Canvas& canvas) {
        for(int i = 0; i< canvas.getHeight()*canvas.getWidth();i ++) {
            canvas[i] = 0x000000;
        }
    }

    Canvas::Canvas(int nw, int nh) {
        w = nw; h = nh;
        buffer = new unsigned char[nw*nh*channels];
        initializedBuffer = true;
    }

    Canvas::Canvas(unsigned char* nbuffer, int nw, int nh) {
        w = nw; h = nh;
        buffer = nbuffer;
        initializedBuffer = false;
    }

    int& Canvas::operator[](int a) {
        return ((int*)buffer)[a];
    }

    int Canvas::getWidth() {
        return w;
    }

    int Canvas::getHeight() {
        return h;
    }

    unsigned char* Canvas::getData() {
        return buffer;
    }
  
    Canvas::~Canvas() {
        if(initializedBuffer)
            delete[] buffer;
    }

    void Canvas::clear(int c = 0x000000) {
        for(int i = 0; i < w*h; i++) {
            ((int*)buffer)[i] = c;
        }
    }

    void Canvas::setPixel(int x, int y, int color) {
        ((int*)buffer)[y * w + x] = color;
    }

    int Canvas::getPixel(int x, int y) {
        return ((int*)buffer)[y * w + x];
    }

    CamParam::CamParam(int w, int h, double fovh = F_PI/2, float nplane = 0.01) {
        screenWidth = w;
        screenHeight = h;
        invtanfovhover2 = 1.f/tan(fovh/2);
        invtanfovvover2 = invtanfovhover2*w/h;
        nearPlane = nplane;
    }

    LineModel::LineModel(int n, int m) {
        numIndices= m;
        numPoints = n;
        indices = new int[2*m];
        points = new Vec3[n];
    }

    LineModel::~LineModel() {
        if(indices)
            delete[] indices;

        if(points)
            delete[] points;
    }

    LineCube::LineCube(float w, float h, float l):LineModel(8,12) {
        for(int i = 0; i < 2; i++) {
            for(int j =0; j < 2; j++) {
                for(int k = 0;k < 2; k++) {
                    points[4*i + 2*j + k] = Vec3((i - 0.5f)*w , (j - 0.5f)*h, (k - 0.5f)*l);
                }
            }
        }
        int u= 0;
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 3; j++) {
                if(! ((1<<j)&i)) {
                    indices[2*u] = i;
                    indices[2*u + 1] = i | (1<<j);
                    u++;
                }
            }
        }
    }

    
    struct Circle {
        falg::Vec2 c;
        float r_sq;

        bool contains(const falg::Vec2& p) {
            return (c - p).sqNorm() <= r_sq;
        }
    };
        
    float rightMostCircleEdge(const falg::Vec2& p0,
                              const falg::Vec2& p1,
                              const falg::Vec2& p2) {
        falg::Vec2 v0 = p1 - p0;
        falg::Vec2 v1 = p2 - p0;
        float t = (v1 - v0) / 2 * (-v1) / (v0.x() * v1.y() - v0.y() * v1.x());

        falg::Vec2 xv0(v0.y(), -v0.x());
            
        falg::Vec2 middle = p0 + v0 / 2 + xv0 * t;

        float r = (middle - p0).norm();

        return middle.x() + r;
            
    }

                
    Circle getCircumCircle(const falg::Vec2& p0,
                           const falg::Vec2& p1,
                           const falg::Vec2& p2) {
        Circle cr;

        falg::Vec2 v0 = p1 - p0;
        falg::Vec2 v1 = p2 - p0;

        float cross = v0.x() * v1.y() - v0.y() * v1.x();
            
        float t = (v1 - v0) / 2 * (-v1) / cross;

        falg::Vec2 xv0(v0.y(), -v0.x());
            
        falg::Vec2 middle = p0 + v0 / 2 + xv0 * t;

        float rsq = (middle - p0).sqNorm();

        cr.c = middle;
        cr.r_sq = rsq;
            
        return cr;
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
