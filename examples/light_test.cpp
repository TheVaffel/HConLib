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
       layout (location = 2) out vec3 position;
       out gl_PerVertex { 
	 vec4 gl_Position;
       };
       void main() {
	 vec4 newPos = transform.mvp * pos;
	 light_vertex = lightVals.mvp * pos;
	 outNormal = lightVals.mvp * normal;
	 gl_Position = newPos;
	 position = newPos.xyz;
       }
       );

const char *fragShaderText =
  GLSL(
       layout (location = 0) in vec4 light_vert;
       layout (location = 1) in vec4 light_normal;
       layout (location = 2) in vec3 position;
       layout (location = 0) out vec4 outColor;
       layout (set = 1, binding = 1) uniform sampler2D depthMap;
       void main() {
	 
	 float dir = max(0, -dot(light_normal, light_vert)/(length(light_normal)*length(light_vert)));
	 vec3 reflect = (light_vert.xyz - dot(light_vert, light_normal) * normalize(light_normal.xyz));
	 float divert = max(0, - dot(reflect, position) / (length(position) * length(reflect)));
	 float visible = textureLod(depthMap, (light_vert.xy/light_vert.w + vec2(1, 1))/2 , 0.0).x
	   > light_vert.z/light_vert.w - 0.00001 * tan(acos(dir))? 1.0: 0.0;
	 outColor = visible * dir * vec4(1, 1, 1, 0.0) + vec4(0.1, 0.1, 0.1, 1.0) + pow(divert, 4)*vec4(1.0, 1.0, 1.0, 0.0);
	 //outColor = dir * vec4(1, 1, 1, 0.0) + vec4(0.1, 0.1, 0.1, 1.0);
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

  WgRSL rsl = wg.createResourceSetLayout({WG_RESOURCE_TYPE_UNIFORM}, {VK_SHADER_STAGE_VERTEX_BIT});

  WgResourceSet cameraSet = wg.createResourceSet(rsl, {&cameraUniform});

  WgShader vertexShader = wg.createVertexShader(vertShaderText);
  WgShader fragmentShader = wg.createFragmentShader(fragShaderText);


  // Depth rendering
  WgUniform lightTransformUniform = wg.createUniform(sizeof(Matrix4));

  WgTexture depthImage = wg.createDepthTexture(width, height);

  WgRSL lightLayout = wg.createResourceSetLayout({WG_RESOURCE_TYPE_UNIFORM, WG_RESOURCE_TYPE_TEXTURE},
						 {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT});

  WgShader idVertexShader = wg.createVertexShader(vertShaderIdText);

  WgPipeline depthPipeline = wg.createDepthPipeline({rsl}, {idVertexShader});
  WgObjectGroup depthObjectGroup(wg, depthPipeline);
  
  WingineFramebuffer depthFramebuffer = wg.createDepthFramebuffer(width, height, depthPipeline);

  WgResourceSet lightTransformSet = wg.createResourceSet(rsl, {&lightTransformUniform});
  WgResourceSet lightSet = wg.createResourceSet(lightLayout, {&lightTransformUniform, &depthImage});

  
  WgPipeline renderPipeline = wg.createPipeline({rsl, lightLayout}, {vertexShader, fragmentShader},
						{WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_4}, true);
  WgObjectGroup renderObjectGroup(wg, renderPipeline);
  
  WgObject cube(3 * 12, {cubeVertexBuffer, cubeNormalBuffer}, cubeIndexBuffer);
  WgObject floor(3 * 2, {floorVertexBuffer, floorNormalBuffer}, floorIndexBuffer);

  wgutil::Model teapot(wg, "teapot.obj", {WG_ATTRIB_TYPE_POSITION, WG_ATTRIB_TYPE_NORMAL});

  WgCamera cam(F_PI/4, wg.getScreenHeight()/((float)wg.getScreenWidth()), 0.1f, 100.0f);
  Vector3 camPos(9, 8, -6);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));


  WgCamera lightCamera(F_PI/3, 9.0f/16.0f, 0.1f, 100.0f);
  Matrix4 lightRot(FLATALG_MATRIX_ROTATION_Y, 0.015f);
  Vector3 lightPos(-8, 7, 2);
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

    cam.setPosition(camPos + 0.3*camPos*sin(0.02f*count));
    Matrix4 cmat = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cmat, sizeof(Matrix4));

    Matrix4 lightMat = lightCamera.getRenderMatrix();
    wg.setUniform(lightTransformUniform, &lightMat, sizeof(Matrix4));

    depthObjectGroup.startRecording(depthFramebuffer);
    //depthObjectGroup.recordRendering(cube, {lightTransformSet});
    depthObjectGroup.recordRendering(teapot, {lightTransformSet});
    depthObjectGroup.recordRendering(floor, {lightTransformSet});
    depthObjectGroup.endRecording();

    wg.copyDepthFromFramebuffer(depthFramebuffer, depthImage.image);
    
    renderObjectGroup.startRecording();
    //renderObjectGroup.recordRendering(cube, {cameraSet, lightSet});
    renderObjectGroup.recordRendering(teapot, {cameraSet, lightSet});
    renderObjectGroup.recordRendering(floor, {cameraSet, lightSet});
    renderObjectGroup.endRecording();

    wg.present();
    
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

  wg.destroyPipeline(renderPipeline);

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

  teapot.destroy();

  return 0;
}
