//Graphics utility library - Haakon Flatval

#ifndef INCLUDED_HGRAF
#define INCLUDED_HGRAF
#include "FlatAlg.hpp"


namespace hg{

  typedef Point3 Color;

  struct Canvas{
  private:
    unsigned char* buffer;
    static const int channels = 4;
    int w, h;
    bool initializedBuffer;
  public:

    Canvas(int nw, int nh);
    Canvas(unsigned char* buffer, int nw, int nh);

    int& operator[](int a);

    int getWidth();
    int getHeight();

    void setPixel(int x, int y, int color);
    int getPixel(int x, int y);
    
    unsigned char* getData();
    void clear(int c);
    ~Canvas();
  
  };

  struct Rectangle {
  private:
    float x, y, w, h;
  public:
    
    Rectangle();
    Rectangle(float x, float y, float w, float h);

    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;

    void setX(float nx);
    void setY(float ny);
    void setWidth(float nw);
    void setHeight(float nh);

    bool contains(const Point2& p) const;
  };

  void drawRectangle(Canvas* canvas, const Rectangle& rectangle, const Color c);
  
  int colorToInt(Color c);

  void drawLine(unsigned char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum);

  void drawLineSafe(unsigned char* buffer, int w, int h, int startx, int starty, int endx, int endy, int colorNum);

  void drawLineSafe(Canvas& canvas, int startx, int starty, int endx, int endy, int colorNum);

  void getLinePoints(int x0, int y0, int x1, int y1, int poss[][2], int& numposs);
  
  struct LineIterator{
    int currPoint[2];
    int ground, notGround;
    int stepGround, stepNotGround;
    int endPoint[2];
    int dd[2];
    int curr;
    LineIterator(int, int);
    LineIterator(int sx, int sy, int ex, int ey);
    LineIterator(const LineIterator&);
    int operator[](int i) const;
    LineIterator operator++(int);
    LineIterator end();
    bool operator==(const LineIterator&);
    bool isFinished() const;
  };
  
  struct CamParam{
  public:
    double invtanfovhover2, invtanfovvover2;
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
  
  void getBoundaryIntersections(const int point[2], const float vector[2], const int boundary[2][2], int endPoints[2][2]);
  void drawLine3D(Canvas& canvas, const CamParam& camparam, const Point3& start, const Point3& end, int color);
  bool moveEndpointsOntoScreen(int& sx, int& sy, int& ex, int& ey, int w, int h);

  void cutLineToZPlane(const Point3& p1, const Point3& p2, float plane, Point3& dst1, Point3& dst2);
  void drawLineModel(Canvas& canvas, const CamParam& camparam, const LineModel& model, const Matrix4&, int);
  void clearCanvas(Canvas& canvas);

  
  

};
#endif // #ifndef INCLUDED_HGRAF