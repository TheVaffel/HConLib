#include <Winval.h>
#include <Wingine.h>
#include <iostream>

using namespace std;

const char *vertShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (std140, binding = 0) uniform bufferVals {\n"
  "    mat4 mvp;\n"
  "} myBufferVals;\n"
  "layout (location = 0) in vec4 pos;\n"
  "layout (location = 1) in vec4 normal;\n"
  "layout (location = 0) out float lightness;\n"
  "out gl_PerVertex { \n"
  "    vec4 gl_Position;\n"
  "};\n"
  "void main() {\n"
  "   vec4 newPos = myBufferVals.mvp * pos;\n"
  "   vec4 addVec = vec4(0.5, 0.5, -5, 0.0);\n"
  //"   gl_Position = newPos + addVec;\n"
  "   gl_Position = newPos\n;"
  //"   gl_Position = pos;"
  "   lightness = dot(-normalize(newPos), normalize(myBufferVals.mvp * normal));\n"
  "}\n";

const char *fragShaderText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (location = 0) in float lightness;\n"
  "layout (location = 0) out vec4 outColor;\n"
  "void main() {\n"
  "  outColor = lightness * vec4(0.8f, 0.8f, 0.8f, 1.0f);\n"
  "  outColor.w = 1.0f;\n"
  "}\n";


const char *vertShaderIdText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (std140, binding = 0) uniform bufferVals {\n"
  "    mat4 mvp;\n"
  "} myBufferVals;\n"
  "layout (location = 0) in vec4 pos;\n"
  "out gl_PerVertex { \n"
  "    vec4 gl_Position;\n"
  "};\n"
  "void main() {\n"
  "   vec4 newPos = myBufferVals.mvp * pos;\n"
  "   gl_Position = newPos\n;"
  "}\n";

const char *fragShaderDepthText =
  "#version 400\n"
  "#extension GL_ARB_separate_shader_objects : enable\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "layout (location = 0) out float depth;\n"
  "void main() {\n"
  "  depth = gl_FragCoord.z;\n"
  "}\n";

const float floor_vertices[] = {
  -2.0f, 0.0f, -2.0f, 1.0f,
  2.0f, 0.0f, -2.0f, 1.0f,
  -2.0f, 0.0f, 2.0f, 1.0f,
  2.0f, 0.0f, 2.0f, 1.0f
};

static const float floor_normals[] =
  { 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f};

const uint32_t floor_indices[] = {
  0, 1, 3,
  0, 3, 2
};

int main(){
  // Creating cube
  int cube_indices[3*12];
  float cube_vertices[4 * 4 * 6];
  float cube_normals[4 * 4 * 6];
  memset(cube_normals, 0, sizeof(float) * 4 * 4 * 6);

  for(int k = 0; k < 3; k++){
    for(int j = 0; j < 2; j++){
      for(int i = 0; i  < 4; i++){
        cube_vertices[4 * 4 * 2 * k + 4 * 4 * j + 4 * i + k] =
          ((i%2 > 0) ^ j ? 0.5f : -0.5f);
        cube_vertices[4 * 4 * 2 * k + 4 * 4 * j + 4 * i + ((k + 1) % 3)] =
          ((i/2 > 0) ? 0.5f : -0.5f);
        cube_vertices[4 * 4 * 2 * k + 4 * 4 * j + 4 * i + ((k + 2) % 3)] =
          0.5f * (j * 2 - 1);
        cube_vertices[4 * 4 * 2 * k + 4 * 4 * j + 4 * i + 3] = 1.0f;

        cube_normals[4 * 4 * 2 * k + 4 * 4 * j + 4 * i + ((k + 2) % 3)] = (j * 2 - 1.0f);
      }
    }
  }
  int quad_vertex_order[] = {0, 1, 2, 2, 1, 3};
  for(int i = 0; i < 6; i++){
    for(int j = 0; j < 6; j++){
      cube_indices[6 * i + j] = i * 4 + quad_vertex_order[j];
    }
  }

  const int width = 1280;
  const int height = 720;

  Winval win(width, height);
  Wingine wg(win);

  // Move cube up
  for(int i = 0; i < 6 * 4; i++){
    cube_vertices[4 * i + 1] += 0.5f;
  }
  printf("Starting\n");
  WgBuffer floorVertexBuffer = wg.createVertexBuffer(4*4*sizeof(float), floor_vertices);
  WgBuffer floorNormalBuffer = wg.createVertexBuffer(4 * 4 * sizeof(float), floor_normals);
  WgBuffer floorIndexBuffer = wg.createIndexBuffer(2 * 3 * sizeof(uint32_t), floor_indices);

  WgBuffer cubeVertexBuffer = wg.createVertexBuffer(4 * 6 * 4 * sizeof(float), cube_vertices);
  WgBuffer cubeNormalBuffer = wg.createVertexBuffer(4 * 6 * 4 * sizeof(float), cube_normals);
  WgBuffer cubeIndexBuffer = wg.createIndexBuffer(3 * 12 * sizeof(uint32_t), cube_indices);

  WgUniform cameraUniform = wg.createUniform(sizeof(Matrix4));

  VkShaderStageFlagBits resourceSetStageBits[] = {VK_SHADER_STAGE_VERTEX_BIT};
  WgResourceType resourceTypes[] = {WG_RESOURCE_TYPE_UNIFORM};
  WgRSL rsl = wg.createResourceSetLayout(1, resourceTypes, resourceSetStageBits);

  WgResource* cameraResource = &cameraUniform;
  WgResourceSet cameraSet = wg.createResourceSet(rsl, &cameraResource);


  WgShader vertexShader = wg.createVertexShader(vertShaderText);
  WgShader fragmentShader = wg.createFragmentShader(fragShaderText);
  WgShader shaders[] = {vertexShader, fragmentShader};

  WingineScene scene(wg);

  WgAttribFormat attribTypes[] = {WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_4};
  scene.addPipeline(rsl, 2, shaders, 2, attribTypes);

  WgBuffer cubeVertexAttribs[] = {cubeVertexBuffer, cubeNormalBuffer};
  WgBuffer floorVertexAttribs[] = {floorVertexBuffer, floorNormalBuffer};
  WgObject cube(3 * 12, 2, cubeVertexAttribs, cubeIndexBuffer, cameraSet);
  WgObject floor(3 * 2, 2, floorVertexAttribs, floorIndexBuffer, cameraSet);

  scene.addObject(cube, 0);
  scene.addObject(floor, 0);

  printf("Starting depth things\n");
  // Depth rendering
  WgUniform lightTransformUniform = wg.createUniform(sizeof(Matrix4));
  WgResource* lightResource = &lightTransformUniform;
  WgResourceSet lightSet = wg.createResourceSet(rsl, &lightResource);

  WgShader idVertexShader = wg.createVertexShader(vertShaderIdText);
  WgShader depthFragShader = wg.createFragmentShader(fragShaderDepthText);

  printf("Creating depth pipeline\n");

  WgShader depthShaders[] = {idVertexShader, depthFragShader};
  WgPipeline depthPipeline = wg.createDepthPipeline(rsl, 2, depthShaders);

  WgObjectGroup depthObjectGroup(wg);
  depthObjectGroup.addObject(cube);
  depthObjectGroup.addObject(floor);

  printf("Creating depth framebuffer\n");
  WingineFramebuffer depthFramebuffer = wg.createDepthFramebuffer(width, height
    , depthPipeline);
  printf("Created framebuffer\n");

  WgCamera cam(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(4, 5, -6);
  cam.setLookAt(camPos,
    Vector3(0, 0, 0),
    Vector3(0, 1, 0));

  wg.setScene(scene);

  WgCamera lightCamera(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 lightPos(-4, 4, -5);
  lightCamera.setLookAt(lightPos,
    Vector3(0, 0, 0),
    Vector3(0, 1, 0));

  int count = 0;

  while(win.isOpen()){
    count++;

    cam.setPosition(camPos + 0.5f*camPos*sin(0.03f*count));
    Matrix4 cmat = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cmat, sizeof(Matrix4));

    Matrix4 lightMat = lightCamera.getRenderMatrix();
    printf("Setting light uniform\n");
    wg.setUniform(lightTransformUniform, &lightMat, sizeof(Matrix4));
    printf("To render object group\n");
    wg.renderObjectGroup(depthObjectGroup);
    wg.renderScene();

    win.sleepMilliseconds(20);
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  wg.destroyUniform(cameraUniform);
  wg.destroyShader(vertexShader);
  wg.destroyShader(fragmentShader);
  wg.destroyResourceSet(cameraSet);
  wg.destroyResourceSetLayout(rsl);
  wg.destroyBuffer(cubeVertexBuffer);
  wg.destroyBuffer(floorVertexBuffer);
  wg.destroyBuffer(floorNormalBuffer);
  wg.destroyBuffer(cubeNormalBuffer);

  wg.destroyBuffer(cubeIndexBuffer);
  wg.destroyBuffer(floorIndexBuffer);

  // Depth things
  wg.destroyShader(idVertexShader);
  wg.destroyShader(depthFragShader);
  wg.destroyUniform(lightTransformUniform);
  wg.destroyResourceSet(lightSet);
  wg.destroyPipeline(depthPipeline);
  wg.destroyFramebuffer(depthFramebuffer);

  return 0;
}
