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

const char *vertShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (std140, binding = 0) uniform bufferVals {\n"
  "    mat4 mvp;\n"
  "} myBufferVals;\n"
  "layout (location = 0) in vec4 pos;\n"
  "layout (location = 1) in vec4 inColor;\n"
  "layout (location = 0) out vec4 outColor;\n"
  "out gl_PerVertex { \n"
  "    vec4 gl_Position;\n"
  "};\n"
  "void main() {\n"
  "   outColor = inColor;\n"
  "   gl_Position = myBufferVals.mvp * pos;\n"
  "}\n";

const char *fragShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (location = 0) in vec4 color;\n"
  "layout (location = 0) out vec4 outColor;\n"
  "void main() {\n"
  "  outColor = color;\n"
  "  //outColor = vec4(1.f, 1.f, 1.f, 0)*(1-gl_FragCoord.w*6) + vec4(0, 0, 0, 1);\n"
  "}\n";

int main(){

  Winval win(1280, 720);
  Wingine wg(win);
  Matrix4 rotation(FLATALG_MATRIX_ROTATION, 0, 0.01f);
  //Matrix4 model = Matrix4(FLATALG_MATRIX_IDENTITY);

  WingineBuffer vertexBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( vertexBuffer, _test_vertices, 3*4*sizeof(float));

  WingineBuffer colorBuffer = wg.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( colorBuffer, _test_colors, 3*4*sizeof(float));

  WingineBuffer indexBuffer = wg.createBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( indexBuffer, _test_indices, 2*3*sizeof(uint32_t));

  WingineBuffer colorBuffer2 = wg.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*4*sizeof(float));
  wg.setBuffer( colorBuffer2, _test_colors2, 3*4*sizeof(float));


  VkShaderStageFlagBits bits[1] = {VK_SHADER_STAGE_VERTEX_BIT};
  WingineUniform cameraUniform = wg.createUniform(sizeof(Matrix4));
  WingineUniformSet cameraSet = wg.createUniformSet(1, &cameraUniform, bits, "Camera");
  
  WingineBuffer vertexAttribs[2] = {vertexBuffer, colorBuffer};
  WingineBuffer vertexAttribs2[2] = {vertexBuffer, colorBuffer2};

  WingineShader vertexShader = wg.createShader(vertShaderText, 1, /*&cameraSet,*/ VK_SHADER_STAGE_VERTEX_BIT);
  WingineShader fragmentShader = wg.createShader(fragShaderText, 0, /*NULL,*/ VK_SHADER_STAGE_FRAGMENT_BIT);

  WingineShader shaders[2] = {vertexShader, fragmentShader};

  WingineScene scene(wg);
  scene.addPipeline(2, shaders, 2); //Num shaders, shaders, num vertex attribs
  WingineRenderObject object1(2, vertexAttribs, indexBuffer, cameraSet); //Num vertex attribs, vertex attribs, index buffer, uniformset
  WingineRenderObject object2(2, vertexAttribs2, indexBuffer, cameraSet);

  scene.addObject(object1, 0);
  scene.addObject(object2, 0);

  wg.setScene(scene);
  
  WingineCamera cam(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(-5, 3, -10);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));
  
  
    
  clock_t start_time = clock();
  int count = 0;

  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.1f*count));
    count++;
    Matrix4 cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.renderScene();
    clock_t current_time = clock();
    long long int diff = current_time - start_time;
    long long w = 1000/60 - 1000*diff/CLOCKS_PER_SEC;
    if(w > 0){
      sleepMilliseconds((int32_t)w);
    }
    start_time = current_time;
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }
  /*WinginePipeline pipeline = wg.createPipeline(2, shaders, 2);
  
  //wg.setCamera(cam);
  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.1f*count));
    Matrix4 cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.render(count%2 == 3?vertexAttribs:vertexAttribs2, indexBuffer, pipeline, true);

    //wg.synchronizeDrawing();
    
    cam.setPosition(camPos + 0.5f*camPos*sin(0.1f*count) + Vector3(3.3f, 0.0f, 0.0f));
    cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.render(count%2 < 3? vertexAttribs:vertexAttribs2, indexBuffer, pipeline, false);
    wg.present();
    
    count++;

    clock_t current_time = clock();

    long long int diff = current_time - start_time;
    long long w = 1000/60 - 1000*diff/CLOCKS_PER_SEC;
    if(w > 0){
      sleepMilliseconds((int32_t)w);
    }
    start_time = current_time;

    //if(win.waitForKey() == WK_ESC){
      //break;
      //}

    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
    }*/
  
  wg.destroyShader(vertexShader);
  wg.destroyShader(fragmentShader);
  wg.destroyUniformSet(cameraSet);
  wg.destroyUniform(cameraUniform);
  
  wg.destroyBuffer(vertexBuffer);
  wg.destroyBuffer(colorBuffer);
  wg.destroyBuffer(indexBuffer);
  wg.destroyBuffer(colorBuffer2);
  
  return 0;
}
