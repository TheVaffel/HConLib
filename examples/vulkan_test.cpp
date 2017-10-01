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

static const float texture_coords[] =
  {1.0f, 1.0f,
   0.0f, 0.0f,
   1.0f, 0.0f};

static const uint32_t _test_indices[] =
  { 0, 1, 2,
    0, 2, 1};

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

const char *texVertShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (std140, binding = 0) uniform bufferVals {\n"
  "    mat4 mvp;\n"
  "} myBufferVals;\n"
  "layout (location = 0) in vec4 pos;\n"
  "layout (location = 1) in vec2 texCoord;\n"
  "layout (location = 0) out vec2 outTexCoord;\n"
  "out gl_PerVertex { \n"
  "    vec4 gl_Position;\n"
  "};\n"
  "void main() {\n"
  "   outTexCoord = texCoord;\n"
  "   gl_Position = myBufferVals.mvp * pos;\n"
  "}\n";

const char* texFragShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (binding = 1) uniform sampler2D tex;\n"
  "layout (location = 0) in vec2 texCoord;\n"
  "layout (location = 0) out vec4 outColor;\n"
  "void main() {\n"
  "  outColor = textureLod(tex, texCoord, 0.0);\n"
  "}\n";

int main(){
  int texWidth = 64;
  int texHeight = 64;
  unsigned char generic_pattern[4*texWidth*texHeight];
  for(int i =0; i < texWidth*texHeight; i++){
    ((int*)generic_pattern)[i] = (i % 8 < 4) ^ ((i/texWidth) % 8 <4) ? 0xFFFFFFFF : 0xFF000000; 
  }
  

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

  WingineBuffer textureCoordBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 3*2*sizeof(float));
  wg.setBuffer(textureCoordBuffer, texture_coords, 2*4*sizeof(float));

  VkShaderStageFlagBits bits[1] = {VK_SHADER_STAGE_VERTEX_BIT};
  VkShaderStageFlagBits textureResourceSetStageBits[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    
  WingineUniform cameraUniform = wg.createUniform(sizeof(Matrix4));
  WingineTexture texture = wg.createTexture(texWidth, texHeight, generic_pattern);
  
  WingineResourceSetLayout resourceLayout = wg.createResourceSetLayout(1, 0, bits);
  WingineResourceSetLayout textureResourceLayout = wg.createResourceSetLayout(1, 1, textureResourceSetStageBits);
  
  WingineResourceSet cameraSet = wg.createResourceSet(resourceLayout, &cameraUniform, NULL);
  WingineResourceSet textureSet = wg.createResourceSet(textureResourceLayout, &cameraUniform, &texture);
  
  WingineBuffer vertexAttribs[2] = {vertexBuffer, colorBuffer};
  WingineBuffer vertexAttribs2[2] = {vertexBuffer, colorBuffer2};
  WingineBuffer textureVertexAttribs[2] = {vertexBuffer, textureCoordBuffer};

  VkFormat attribTypes[] = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
  VkFormat attribTypesTexture[] = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32_SFLOAT};

  WingineShader vertexShader = wg.createShader(vertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
  WingineShader fragmentShader = wg.createShader(fragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

  WingineShader textureVertexShader = wg.createShader(texVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
  WingineShader textureFragmentShader = wg.createShader(texFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

  WingineShader shaders[2] = {vertexShader, fragmentShader};
  WingineShader textureShaders[2] = {textureVertexShader, textureFragmentShader};

  WingineScene scene(wg);
  scene.addPipeline(resourceLayout, 2, shaders, 2, attribTypes); //pipelineLayout, num shaders, shaders, num vertex attribs
  scene.addPipeline(textureResourceLayout, 2, textureShaders, 2, attribTypesTexture);
  WingineRenderObject object1(2, vertexAttribs, indexBuffer, cameraSet); //Num vertex attribs, vertex attribs, index buffer, resourceset
  WingineRenderObject object2(2, vertexAttribs2, indexBuffer, cameraSet);
  WingineRenderObject object3(2, textureVertexAttribs, indexBuffer, textureSet);

  scene.addObject(object1, 0); //Object, pipeline number
  //scene.addObject(object2, 0);
  scene.addObject(object3, 1);

  wg.setScene(scene);
  
  WingineCamera cam(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(-5, 3, -10);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));
  
  
    
  clock_t start_time = clock();
  int count = 0;

  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.01f*count));
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
  
  wg.destroyTexture(texture);
  wg.destroyUniform(cameraUniform);
  
  wg.destroyShader(vertexShader);
  wg.destroyShader(fragmentShader);
  wg.destroyShader(textureVertexShader);
  wg.destroyShader(textureFragmentShader);
  
  wg.destroyResourceSet(cameraSet);
  wg.destroyResourceSet(textureSet);

  wg.destroyResourceSetLayout(resourceLayout);
  wg.destroyResourceSetLayout(textureResourceLayout);
  
  wg.destroyBuffer(vertexBuffer);
  wg.destroyBuffer(colorBuffer);
  wg.destroyBuffer(indexBuffer);
  wg.destroyBuffer(colorBuffer2);
  wg.destroyBuffer(textureCoordBuffer);
  
  return 0;
}
