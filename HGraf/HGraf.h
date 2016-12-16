//Graphics utility library - Haakon Flatval

#ifndef INCLUDED_HGRAF
#define INCLUDED_HGRAF
#include "FlatAlg/FlatAlg.h"
#define _USE_MATH_DEFINES
#include <cmath>

typedef Point3 Color;

struct Canvas{
private:
  char* buffer;
  static const int channels = 4;
  int w, h;
public:

  Canvas(int nw, int nh);

  int& operator[](int a);

  int getWidth();
  int getHeight();

  char* getData();
  void clear(int c);
  ~Canvas();
  
};

struct CamParam{
public:
  double tanfovhover2, tanfovvover2;
  int screenWidth, screenHeight;
  float nearPlane;
  CamParam(int w, int h, double fovh, float nplane);
};

struct LineModel{
  Point3* points;
  int* indices;
  int numPoints;
  int numIndices;
  LineModel(int n, int m);
  ~LineModel();
  
};

struct LineCube: LineModel{
 LineCube(float w, float h, float l);
};

namespace hg{
  int colorToInt(Color c);

  void drawLine(char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum);

  void drawLineSafe(char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum);

  void drawLineSafe(Canvas& canvas, int startx, int starty, int endx, int endy, int colorNum);

  void getLinePoints(int x0, int y0, int x1, int y1, int poss[][2], int& numposs);

  void getBoundaryIntersections(const int point[2], const float vector[2], const int boundary[2][2], int endPoints[2][2]);
  void drawLine3D(Canvas& canvas, const CamParam& camparam, const Point3& start, const Point3& end);
  bool moveEndpointsOntoScreen(int& sx, int& sy, int& ex, int& ey, int w, int h);

  void cutLineToZPlane(const Point3& p1, const Point3& p2, float plane, Point3& dst1, Point3& dst2);
  void drawLineModel(Canvas& canvas, const CamParam& camparam, const LineModel& model);
  void clearCanvas(Canvas& canvas);
};
#endif // #ifndef INCLUDED_HGRAF

#ifdef HGRAF_IMPLEMENTATION

#ifndef FLATALG_IMPLEMENTATION
#define FLATALG_IMPLEMENTATION
#include <FlatAlg/FlatAlg.h>
#endif

Canvas::Canvas(int nw, int nh){
  w = nw; h = nh;
  buffer = new char[nw*nh*channels];
}

int& Canvas::operator[](int a){
  return ((int*)buffer)[a];
}

int Canvas::getWidth(){
  return w;
}

int Canvas::getHeight(){
  return h;
}

char* Canvas::getData(){
  return buffer;
}
  
Canvas::~Canvas(){
  delete[] buffer;
}

void Canvas::clear(int c = 0x000000){
  for(int i = 0; i < w*h; i++){
    ((int*)buffer)[i] = c;
  }
}

CamParam::CamParam(int w, int h, double fovh = M_PI/2, float nplane = 0.01){
  screenWidth = w;
  screenHeight = h;
  tanfovhover2 = tan(fovh/2);
  tanfovvover2 = tanfovhover2/w*h;
  nearPlane = nplane;
}

LineModel::LineModel(int n, int m){
  numIndices= m;
  numPoints = n;
  indices = new int[2*m];
  points = new Point3[n];
}

LineModel::~LineModel(){
  if(indices)
    delete[] indices;

  if(points)
    delete[] points;
}

LineCube::LineCube(float w, float h, float l):LineModel(8,12){
  for(int i = 0; i < 2; i++){
    for(int j =0; j < 2; j++){
      for(int k = 0;k < 2; k++){
	points[4*i + 2*j + k] = Point3((i - 0.5f)*w , (j - 0.5f)*h, (k - 0.5f)*l);
      }
    }
  }
  int u= 0;
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 3; j++){
      if(! ((1<<j)&i)){
	indices[2*u] = i;
	indices[2*u + 1] = i | (1<<j);
	u++;
      }
    }
  }
}

namespace hg{
  inline int _signum(int a){
    return (a > 0) - (a < 0);
  }
  
  inline int colorToInt(Color c){
    return (((int)c[0])<<16) | (((int)c[1])<<8) | ((int)c[2]);
  }

  //An implementation of Bresenham's algorithm
  void drawLine(char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum){
    
    int dx = endx - startx, dy = endy - starty;
    
    if(dx == 0 && dy == 0){
      ((int*)buffer)[w*starty + startx] = colorNum;
      return;
    }
    int pssx, psex, pssy;
    int stepx, stepy;
  
    int xind = 0, yind = 1;
    int err = 0;
    int num = 0;

    if(std::abs(dx) < std::abs(dy)){
      pssx = starty, psex = endy, pssy = startx;
      int a = dx;
      dx = dy;
      dy = a;
      xind = 1; yind = 0;
    }else{
      pssx = startx, psex = endx, pssy = starty;
    }

    if(dx < 0){
      stepx = -1;
      dx = -dx;
    }else{
      stepx = 1;
    }

    if(dy < 0){
      dy = -dy;
      stepy = -1;
    }else{
      stepy = 1;
    }

    int ny = pssy;
    if(yind){
      for(int nx = pssx; nx != psex; nx += stepx){
	((int*)buffer)[w*ny + nx] = colorNum;
	err += dy;
	if(err > dx){
	  err-=dx;
	  ny += stepy;
	}
      }
    }else{
      for(int nx = pssx; nx != psex; nx += stepx){
	((int*)buffer)[w*nx + ny] = colorNum;
	err += dy;
	if(err > dx){
	  err-=dx;
	  ny += stepy;
	}
      }
    }
  }

  inline int _cross(int x1, int y1, int x2, int y2){
    return x1*y2 - x2*y1;
  }

  //return whether it is possible
  
  bool moveEndpointsOntoScreen(int& sx, int& sy, int& ex, int& ey, int w, int h){
    int dx;
    int dy;
    
    int u = _signum(_cross(ex - sx, ey - sy, ex - 0, ey - 0));
    if((sx < 0 ||sy < 0 || sx >=w || sy >= h) && (ex < 0 || ey < 0 || ex >=w || ey >= h) &&
       _signum(_cross(ex - sx, ey - sy, ex - 0, ey - h)) == u &&
       _signum(_cross(ex - sx, ey - sy, ex - w, ey - 0)) == u &&
       _signum(_cross(ex - sx, ey - sy, ex - w, ey - h)) == u){
      return false;
    }
    
    if(sx < 0){
      sy = (int)(((float)-sx)/(ex - sx)*(ey - sy)) + sy;
      sx = 0;
    }else if(sx >=w){
      sy = (int)(((float)w-sx - 1)/(ex-sx)*(ey - sy)) + sy;
      sx = w - 1;
    }

    if(sy < 0){
      sx =(int)(((float)-sy)/(ey - sy)*(ex - sx)) + sx;
      sy = 0;
    }else if(sy >= h){
      sx = (int)(((float)h - sy -1)/(ey - sy)*(ex - sx)) + sx;
      sy  = h - 1;
    }

    if(ex < 0){
      ey = (int)(((float)-ex)/(sx - ex)*(sy - ey)) + ey;
      ex = 0;
    }else if(ex >=w){
      ey = (int)(((float)w-ex - 1)/(sx-ex)*(sy - ey)) + ey;
      ex = w - 1;
    }

    if(ey < 0){
      ex =(int)(((float)-ey)/(sy - ey)*(sx - ex)) + ex;
      ey = 0;
    }else if(ey >= h){
      ex = (int)(((float)h - ey -1)/(sy - ey)*(sx - ex)) + ex;
      ey  = h - 1;
    }

    return true;
  }

  
  inline void drawLineSafe(char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum){
    if(!moveEndpointsOntoScreen(startx, starty, endx, endy, w, h)){
      return;
    }

    drawLine(buffer, w, h, startx, starty, endx, endy, colorNum);
  }

  inline void drawLineSafe(Canvas& canvas, int startx, int starty, int endx, int endy, int colorNum){
    drawLineSafe(canvas.getData(), canvas.getWidth(), canvas.getHeight(), startx, starty, endx, endy, colorNum);
  }


  //Similar to the one above, but returns all line points
  void getLinePoints(int x0, int y0, int x1, int y1, int poss[][2], int& numposs){
    int dx = x1 - x0, dy = y1 - y0;
    int pssx, psex, pssy;
    int stepx, stepy;
  
    int xind = 0, yind = 1;
    int num = 0;
    int err = 0;

    if(std::abs(dx) < std::abs(dy)){
      pssx = y0, psex = y1, pssy = x0;
      int a = dx;
      dx = dy;
      dy = a;
      xind = 1; yind = 0;
    }else{
      pssx = x0, psex = x1, pssy = y0;
    }

    if(dx < 0){
      stepx = -1;
      dx = -dx;
    }else{
      stepx = 1;
    }

    if(dy < 0){
      dy = -dy;
      stepy = -1;
    }else{
      stepy = 1;
    }

    int ny = pssy;

    for(int nx = pssx; nx != psex; nx += stepx){
      poss[num][xind] = nx;
      poss[num++][yind] = ny;
      err += dy;
      if(err > dx){
	err-=dx;
	ny += stepy;
      }
    }
    numposs = num;
  }

  void getBoundaryIntersections(const int point[2], const float vector[2], const int boundary[2][2], int endPoints[2][2]){

    int right = boundary[1][0] - point[0], left = boundary[0][0] - point[0],
      up = boundary[0][1] - point[1], down = boundary[1][1] - point[1];

    float dux1 = right*vector[0],
      duy1 = down*vector[1]; //Kinda meaningless variables, but with right signs
    //Also, theoretically faster than division

    if(dux1*duy1 <0){
      int temp = right;
      right = left;
      left = temp;
    }

    if(std::abs(right*vector[1]) < std::abs(down*vector[0])){
      endPoints[0][0] = point[0] + right;
      endPoints[0][1] = point[1] + right*vector[1]/vector[0];
    }else{
      endPoints[0][0] = point[0] + down*vector[0]/vector[1];
      endPoints[0][1] = point[1] + down;
    }

    if(std::abs(left*vector[1]) < std::abs(up*vector[0])){
      endPoints[1][0] = point[0] + left;
      endPoints[1][1] = point[1] + left*vector[1]/vector[0];
    }else{
      endPoints[1][0] = point[0] + up*vector[0]/vector[1];
      endPoints[1][1] = point[1] + up;
    }
  }

  void cutLineToZPlane(const Point3& p1, const Point3& p2, float plane, Point3& dst1, Point3& dst2){
    Vector3 v = p2 - p1;

    if(p1.z > plane){
      v.normalize();
      dst1 = p1 + v*(p1.z - plane);
    }else dst1 = p1;

    if(p2.z > plane){
      v.normalize();
      dst2 = p2 - v*(p2.z - plane);
    }else dst2 = p2;
  }


  void drawLine3D(Canvas& canvas, const CamParam& camparam, const Point3& start, const Point3& end, int color){
    Point3 nstart, nend;

    if(start.z > -camparam.nearPlane && end.z > -camparam.nearPlane){
      return;
    }
    
    cutLineToZPlane(start, end, camparam.nearPlane, nstart, nend);

    int ps[2] = {(int)((-nstart.x/nstart.z + camparam.tanfovhover2)*camparam.screenWidth)/2,
		 (int)((nstart.y/nstart.z + camparam.tanfovvover2)*camparam.screenHeight)/2};
    int pe[2] = {(int)((-nend.x/nend.z + camparam.tanfovhover2)*camparam.screenWidth)/2,
		 (int)((nend.y/nend.z + camparam.tanfovvover2)*camparam.screenHeight)/2};
    drawLineSafe(canvas, ps[0], ps[1], pe[0], pe[1], color);
  }
  
  void drawLineModel(Canvas& canvas, const CamParam& camparam, const LineModel& model, Matrix4 mat, int color){
    Point3* p = new Point3[model.numPoints];
    for(int i = 0; i< model.numPoints; i++){
      p[i] = mat*model.points[i];
    }
    for(int i = 0; i< model.numIndices; i++){
      drawLine3D(canvas, camparam, p[model.indices[2*i]], p[model.indices[2*i + 1]], color);
      //std::cout<<p[model.indices[2*i]].str()<<" "<< p[model.indices[2*i + 1]].str()<<std::endl;
    }
    delete[] p;
  }

  void clearCanvas(Canvas& canvas){
    for(int i = 0; i< canvas.getHeight()*canvas.getWidth();i ++){
      canvas[i] = 0x000000;
    }
  }
};

#endif //#ifdef HGRAF_IMPLEMENTATION
