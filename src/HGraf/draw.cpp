#include <HGraf/draw.hpp>

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

    void drawRectangle(Canvas* canvas, const Rectangle& rectangle, const Color c) {
        int ic = colorToInt(c);

        int ix = (int)rectangle.min.x(), iy = (int)rectangle.min.y();
        int iw = (int)(rectangle.getDims().x()), ih = (int)(rectangle.getDims().y());

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
        return (255 << 24) | (((int)(c[0] * 255)) << 16) | (((int)(c[1] * 255)) << 8) | ((int)(c[2] * 255));
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
        return x1 * y2 - x2*y1;
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
        if ((vector[0] > 0 && vector[1] < 0) || (vector[1] > 0 && vector[0] < 0)) {
            int temp = right;
            right = left;
            left = temp;
        }

        float a, b;

        if ((a = std::abs(right*vector[1])) < (b = std::abs(down*vector[0]))) {
            endPoints[0][0] = point[0] + right;
            endPoints[0][1] = point[1] + (int)(right*vector[1]/vector[0]);
        } else if(a > b) {
            endPoints[0][0] = point[0] + (int)(down*vector[0]/vector[1]);
            endPoints[0][1] = point[1] + down;
        } else {
            endPoints[0][0] = point[0] + right;
            endPoints[0][1] = point[1] + down;
        }

        if ((a = std::abs(left*vector[1])) < (b = std::abs(up*vector[0]))) {
            endPoints[1][0] = point[0] + left;
            endPoints[1][1] = point[1] + (int)(left*vector[1]/vector[0]);
        } else if(a > b) {
            endPoints[1][0] = point[0] + (int)(up*vector[0]/vector[1]);
            endPoints[1][1] = point[1] + up;
        } else {
            endPoints[1][0] = point[0] + left;
            endPoints[1][1] = point[1] + up;
        }
    }

    void cutLineToZPlane(const Vec3& p1, const Vec3& p2, float plane, Vec3& dst1, Vec3& dst2) {
        Vec3 v = p2 - p1;

        if (p1.z() > -plane) {
            dst1 = p1 - v*((p1.z() + plane)/v.z());
        } else {
            dst1 = p1;
        }

        if (p2.z() > -plane) {
            dst2 = p2 - v * ((p2.z() + plane) / v.z());
        } else {
            dst2 = p2;
        }
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
            if(std::abs(ps[0]) > canvas.getWidth() || std::abs(ps[1]) > canvas.getHeight()) {
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
            for(int j = 0; j < 2; j++) {
                for(int k = 0; k < 2; k++) {
                    points[4 * i + 2 * j + k] = Vec3((i - 0.5f) * w, (j - 0.5f) * h, (k - 0.5f) * l);
                }
            }
        }

        int u= 0;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 3; j++) {
                if (! ((1 << j) & i)) {
                    indices[2 * u] = i;
                    indices[2 * u + 1] = i | (1 << j);
                    u++;
                }
            }
        }
    }

};
