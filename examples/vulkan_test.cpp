#include <Winval.h>
#include <Wingine.h>
#include <iostream>

#ifdef WIN32
void sleepMilliseconds(int u){
  Sleep(u);
}
#else //WIN32
#include <unistd.h>
void sleepMilliseconds(int u){
  usleep(u * 1000);
}
#endif //WIN32

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
  Matrix4 rotation(FLATALG_MATRIX_ROTATION, 0, 0.01f);
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
  WingineCamera cam(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(-5, 3, -10);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));
  
  wg.setCamera(cam);
  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.05f*count));
    wg.setCamera(cam);
    model = rotation*model;
    //Matrix4 usableMvp = ~mvp;
    //updateMVP(usableMvp);
    //WingineBuffer* currBuff = (count%2 == 0)?&colorBuffer:&colorBuffer2;
    WingineBuffer* currBuff = &colorBuffer;
    wg.renderColor(vertexBuffer, *currBuff, indexBuffer, model, true);
    count++;

    clock_t current_time = clock();

    long long int diff = current_time - start_time;
    long long w = 1000/60 - 1000*diff/CLOCKS_PER_SEC;
    if(w > 0){
      sleepMilliseconds((int32_t)w);
    }
    start_time = current_time;

    if(win.waitForKey() == WK_ESC){
      break;
    }
  }

  wg.destroyBuffer(vertexBuffer);
  wg.destroyBuffer(colorBuffer);
  wg.destroyBuffer(indexBuffer);
  wg.destroyBuffer(colorBuffer2);
  
  return 0;
}
