#include <Winval_XCB.h>
#include <Wingine.h>
#include <iostream>

using namespace std;

static const float _test_vertices[] =
  { -1.0f, 0.0f, 0.f, 1.0f,
    1.0f, 0.0f, 0.f, 1.0f,
    0.0f, 1.0f, 0.f, 1.0f};

static const float _test_colors[] =
  { 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f};
    
static const float _test_colors2[] =
  { 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f};
static const uint32_t _test_indices[] =
  { 0, 1, 2,
    0, 2, 1
  };

int main(){
  
  Winval win(1280, 720);
  Wingine wg(win);
  Matrix4 rotation(FLATALG_MATRIX_ROTATION, 0, 0.01);
  Matrix4 model = Matrix4(FLATALG_MATRIX_IDENTITY);

  WingineBuffer vertexBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( vertexBuffer, _test_vertices, 3*4*sizeof(float));

  WingineBuffer colorBuffer = wg.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( colorBuffer, _test_colors, 3*4*sizeof(float));

  WingineBuffer indexBuffer = wg.createBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( indexBuffer, _test_indices, 2*3*sizeof(uint32_t));

  WingineBuffer colorBuffer2 = wg.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( colorBuffer2, _test_colors2, 3*4*sizeof(float));
  
  clock_t start_time = clock();
  int count = 0;
  WingineCamera cam(M_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(-5, 3, -10);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));
  
  wg.setCamera(cam);
  while(1){
    cam.setPosition(camPos + 0.2*camPos*sin(0.01*count));
    wg.setCamera(cam);
    model = rotation*model;
    //Matrix4 usableMvp = ~mvp;
    //updateMVP(usableMvp);
    wg.renderColor(vertexBuffer, count%2 == 0?colorBuffer:colorBuffer2, indexBuffer, model);
    
    cout<<count%2<<endl;
    count++;
    
    clock_t current_time = clock();

    long long int diff = current_time - start_time;
    long long w = 1000000/60 - 1000000*diff/CLOCKS_PER_SEC;
    if(w > 0){
      usleep(1000000/60 - 1000000*diff/CLOCKS_PER_SEC);
    }
    start_time = current_time;
    
    win.waitForKey();
  }

  return 0;
}
