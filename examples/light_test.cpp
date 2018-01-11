#include <Winval.h>
#include <Wingine.h>
#include <iostream>

using namespace std;

const char *vertShaderText =
  GLSL(
       layout (std140, binding = 0) uniform bufferVals {
	 mat4 mvp;
       } transform;
       layout (std140, set = 1, binding = 0) uniform ll {
	 mat4 mvp;
       } lightVals;
       layout (location = 0) in vec4 pos;
       layout (location = 1) in vec4 normal;
       layout (location = 0) out vec4 light_vertex;
       layout (location = 1) out vec4 outNormal;
       out gl_PerVertex { 
	 vec4 gl_Position;
       };
       void main() {
	 vec4 newPos = transform.mvp * pos;
	 light_vertex = lightVals.mvp * pos;
	 outNormal = lightVals.mvp * normal;
	 gl_Position = newPos;
       }
       );

const char *fragShaderText =
  GLSL(
       layout (location = 0) in vec4 light_vert;
       layout (location = 1) in vec4 light_normal;
       layout (location = 0) out vec4 outColor;
       layout (set = 1, binding = 1) uniform sampler2D depthMap;
       void main() {
	 float visible = textureLod(depthMap, (light_vert.xy/light_vert.w + vec2(1, 1))/2 , 0.0).x
	   > light_vert.z/light_vert.w - 0.0001? 1.0: 0.0;
	 float dir = max(0,- dot(light_normal, light_vert)/(length(light_normal)*length(light_vert)));
	 outColor = visible * dir * vec4(1, 1, 1, 0.0) + vec4(0.1, 0.1, 0.1, 1.0);
       }
       );


const char *vertShaderIdText =
  GLSL(
       layout (std140, binding = 0) uniform bufferVals {
	 mat4 mvp;
       } transform;
       layout (location = 0) in vec4 pos;
       out gl_PerVertex { 
	 vec4 gl_Position;
       };
       void main() {
	 vec4 newPos = transform.mvp * pos;
	 gl_Position = newPos;
       }
       );

const float floor_vertices[] = {
  -5.0f, 0.0f, -5.0f, 1.0f,
  5.0f, 0.0f, -5.0f, 1.0f,
  -5.0f, 0.0f, 5.0f, 1.0f,
  5.0f, 0.0f, 5.0f, 1.0f
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


  WgBuffer cubeVertexAttribs[] = {cubeVertexBuffer, cubeNormalBuffer};
  WgBuffer floorVertexAttribs[] = {floorVertexBuffer, floorNormalBuffer};



  // Depth rendering
  WgUniform lightTransformUniform = wg.createUniform(sizeof(Matrix4));

  WgTexture depthImage = wg.createDepthTexture(width, height);
  VkShaderStageFlagBits lightResourceSetStageBits[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
  WgResourceType lightResourceTypes[] = {WG_RESOURCE_TYPE_UNIFORM, WG_RESOURCE_TYPE_TEXTURE};
  WgResource* lightResources[] = {&lightTransformUniform, &depthImage};
  WgResource* lightTransformResource[] = {&lightTransformUniform};


  WgRSL lightLayout = wg.createResourceSetLayout(2, lightResourceTypes, lightResourceSetStageBits);

  WgShader idVertexShader = wg.createVertexShader(vertShaderIdText);

  WgShader depthShaders[] = {idVertexShader};
  WgRSL depthSetLayouts[] = {rsl, lightLayout};
  WgPipeline depthPipeline = wg.createDepthPipeline(rsl, 1, depthShaders);
  WingineFramebuffer depthFramebuffer = wg.createDepthFramebuffer(width, height, depthPipeline);

  WgResourceSet lightTransformSet = wg.createResourceSet(rsl, lightTransformResource);
  WgResourceSet lightSet = wg.createResourceSet(lightLayout, lightResources);

  WgResourceSet lightSets[] = {cameraSet, lightSet};
  WgObject lightCube(3 * 12, 2, cubeVertexAttribs, cubeIndexBuffer, lightTransformSet);
  WgObject lightFloor(3 * 2, 2, floorVertexAttribs, floorIndexBuffer, lightTransformSet);


  scene.addPipeline(2, depthSetLayouts, 2, shaders, 2, attribTypes);

  WgObject cube(3 * 12, 2, cubeVertexAttribs, cubeIndexBuffer, 2, lightSets);
  WgObject floor(3 * 2, 2, floorVertexAttribs, floorIndexBuffer, 2, lightSets);


  scene.addObject(floor, 0);
  scene.addObject(cube, 0);

  WgObjectGroup depthObjectGroup(wg, depthPipeline);
  depthObjectGroup.addObject(lightCube);
  depthObjectGroup.addObject(lightFloor);

  WgCamera cam(F_PI/4, wg.getScreenHeight()/((float)wg.getScreenWidth()), 0.1f, 100.0f);
  Vector3 camPos(4, 5, -6);
  cam.setLookAt(camPos,
    Vector3(0, 0, 0),
    Vector3(0, 1, 0));

  wg.setScene(scene);

  WgCamera lightCamera(F_PI/3, 9.0f/16.0f, 0.1f, 100.0f);
  Matrix4 lightRot(FLATALG_MATRIX_ROTATION_Y, 0.02f);
  Vector3 lightPos(-3, 7, 5);
  lightCamera.setLookAt(lightPos,
    Vector3(0, 0, 0),
    Vector3(0, 1, 0));

  int count = 0;

  while(win.isOpen()){
    count++;

    lightPos = lightRot * lightPos;
    lightCamera.setLookAt(lightPos,
    Vector3(0, 0, 0),
    Vector3(0, 1, 0));

    cam.setPosition(camPos + 0.3f*camPos*sin(0.03f*count));
    Matrix4 cmat = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cmat, sizeof(Matrix4));

    Matrix4 lightMat = lightCamera.getRenderMatrix();
    wg.setUniform(lightTransformUniform, &lightMat, sizeof(Matrix4));

    wg.renderObjectGroup(depthObjectGroup, depthFramebuffer);

    wg.copyDepthFromFramebuffer(depthFramebuffer, depthImage.image);
    wg.renderScene();

    win.sleepMilliseconds(50);
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
  wg.destroyUniform(lightTransformUniform);
  wg.destroyPipeline(depthPipeline);
  wg.destroyFramebuffer(depthFramebuffer);

  wg.destroyTexture(depthImage);
  wg.destroyResourceSetLayout(lightLayout);
  wg.destroyResourceSet(lightSet);

  wg.destroyObject(cube);
  wg.destroyObject(floor);

  wg.destroyObject(lightCube);
  wg.destroyObject(lightFloor);

  return 0;
}
