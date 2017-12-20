#include <Winval.h>
#include <Wingine.h>
#include <iostream>

using namespace std;

static const float test_vertices[] =
  { 0.0f, 0.0f, 0.f, 1.0f,
    1.0f, 0.0f, 0.f, 1.0f,
    0.0f, 1.0f, 0.f, 1.0f,
    1.0f, 1.0f, 0.f, 1.0f};

static const float test_colors[] =
  { 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f};

static const float texture_coords[] =
  {0.0f, 0.0f,
   1.0f, 0.0f,
   0.0f, 1.0f,
   1.0f, 1.0f};

static const uint32_t test_indices[] =
  { 0, 1, 3,
    0, 3, 2};

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

const char* computeShaderText =
  "#version 450\n"
  "#extension GL_ARB_shading_language_420pack : enable\n"
  "#extension GL_ARB_compute_shader : enable\n"

  "layout(binding = 0, rgba32f) uniform image2D outputs;\n"

  "layout (local_size_x = 16, local_size_y = 16) in;\n"
  "void main() {\n"
  "  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);\n"
  "  imageStore(outputs, coords, vec4(coords/64.0, 0, 1));\n"
  "}";


int main(){
  const int texWidth = 64;
  const int texHeight = 64;
  unsigned char generic_pattern[4*texWidth*texHeight];
  for(int i = 0; i < texHeight; i++){
    for(int j = 0; j < texWidth; j++){
      ((int*)generic_pattern)[i*texWidth + j] = (((i/8)%2 == 0) ^ ((j/8)%2 == 0)) ? 0xFFFFFFFF : 0xFF000000;
    }
  }

  //Dirty trick for creating cube:
  int cubeNumbers[8] = {0, 1, 3, 2, 6, 4, 5, 7};

  float cube_vertices[4*8];
  float cube_tex_coords[2*8];
  for(int i = 0; i < 8; i++){
    int pop = 0;
    for(int j = 0; j < 3; j++){
      if(cubeNumbers[i] & (1 << j)){
	cube_vertices[4*i + j] = 0.5f;
	pop++;
      }else{
	cube_vertices[4*i + j] = -0.5;
      }
    }
    cube_vertices[4*i + 3] = 1.0f;
    cube_tex_coords[2*i] = pop & 1 ? 1.0f : 0.0f;
    cube_tex_coords[2*i] = pop & 2 ? 1.0f : 0.0f;
  }

  int cube_indices[3*12];
  int pivots[2] = {0, 7};
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 6; j++){
      cube_indices[3*(i*6 + j)] = pivots[i];
      cube_indices[3*(i*6 + j) + (i?2:1)] = 1 + j;
      cube_indices[3*(i*6 + j) + (i?1:2)] = 1 + ((j + 1) % 6);
    }
  }

  Winval win(1280, 720);
  Wingine wg(win);


  WingineBuffer vertexBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 4*4*sizeof(float));
  wg.setBuffer( vertexBuffer, test_vertices, 4*4*sizeof(float));

  WingineBuffer colorBuffer = wg.createBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 4*4*sizeof(float));
  wg.setBuffer( colorBuffer, test_colors, 4*4*sizeof(float));

  WingineBuffer indexBuffer = wg.createBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 3*2*sizeof(int32_t));
  wg.setBuffer( indexBuffer, test_indices, 2*3*sizeof(uint32_t));

  WingineBuffer textureCoordBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 4*2*sizeof(float));
  wg.setBuffer(textureCoordBuffer, texture_coords, 2*4*sizeof(float));


  WingineBuffer cubeVertexBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 8*4*sizeof(float));
  wg.setBuffer(cubeVertexBuffer, cube_vertices, 8*4*sizeof(float));

  WingineBuffer cubeIndexBuffer = wg.createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 12*3*sizeof(int));
  wg.setBuffer(cubeIndexBuffer, cube_indices, 12*3*sizeof(int));

  WingineBuffer cubeTextureCoordBuffer = wg.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 8*2*sizeof(float));
  wg.setBuffer(cubeTextureCoordBuffer, cube_tex_coords, 8*2*sizeof(float));


  VkShaderStageFlagBits bits[1] = {VK_SHADER_STAGE_VERTEX_BIT};
  VkShaderStageFlagBits textureResourceSetStageBits[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  WingineUniform cameraUniform = wg.createUniform(sizeof(Matrix4));
  WingineTexture texture = wg.createTexture(texWidth, texHeight, generic_pattern);
  WingineUniform offsetUniform = wg.createUniform(sizeof(Matrix4));

  VkDescriptorType desc[] = {WINGINE_RESOURCE_UNIFORM};
  WingineResourceSetLayout resourceLayout = wg.createResourceSetLayout(1, desc, bits);

  VkDescriptorType textDescriptorTypes[] = {WINGINE_RESOURCE_UNIFORM, WINGINE_RESOURCE_TEXTURE};
  WingineResourceSetLayout textureResourceLayout = wg.createResourceSetLayout(2, textDescriptorTypes, textureResourceSetStageBits);

  WingineResource* cameraUniformResource = &cameraUniform;
  WingineResourceSet cameraSet = wg.createResourceSet(resourceLayout, &cameraUniformResource);
  WingineResource* resources[] = {&offsetUniform, &texture};
  WingineResourceSet textureSet = wg.createResourceSet(textureResourceLayout, resources);

  WingineBuffer vertexAttribs[2] = {vertexBuffer, colorBuffer};
  WingineBuffer textureVertexAttribs[2] = {vertexBuffer, textureCoordBuffer};

  VkFormat attribTypes[] = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
  VkFormat attribTypesTexture[] = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32_SFLOAT};

  WingineShader vertexShader = wg.createShader(vertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
  WingineShader fragmentShader = wg.createShader(fragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

  WingineShader textureVertexShader = wg.createShader(texVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
  WingineShader textureFragmentShader = wg.createShader(texFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

  // #onlycomputethings
  desc[0] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  bits[0] = VK_SHADER_STAGE_COMPUTE_BIT;

  WingineResourceSetLayout computeLayout = wg.createResourceSetLayout(1, desc, bits);
  WingineKernel kernel = wg.createKernel(computeShaderText, computeLayout);

  WingineImage im = wg.createImage(64, 64, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

  wg.setLayout(im, VK_IMAGE_LAYOUT_GENERAL);
  WingineResource* computeImageResource[] = {&im};
  WingineResourceSet kernelResources = wg.createResourceSet(computeLayout,computeImageResource);
  wg.executeKernel(kernel, kernelResources, 64, 64, 1);


  wg.copyImage(64, 64, im.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, texture.image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  WingineShader shaders[2] = {vertexShader, fragmentShader};
  WingineShader textureShaders[2] = {textureVertexShader, textureFragmentShader};

  WingineScene scene(wg);
  scene.addPipeline(resourceLayout, 2, shaders, 2, attribTypes); //pipelineLayout, num shaders, shaders, num vertex attribs
  scene.addPipeline(textureResourceLayout, 2, textureShaders, 2, attribTypesTexture);
  WingineRenderObject object1(6, 2, vertexAttribs, indexBuffer, cameraSet); //Num drawIndices, num vertex attribs, vertex attribs, index buffer, resourceset
  WingineRenderObject object3(6, 2, textureVertexAttribs, indexBuffer, textureSet);

  WingineBuffer cubeVertexAttribs[2] = {cubeVertexBuffer, cubeTextureCoordBuffer};
  WingineRenderObject cubeObject(3*12, 2, cubeVertexAttribs, cubeIndexBuffer, textureSet);

  scene.addObject(object1, 0); //Object, pipeline number
  //scene.addObject(object3, 1);

  scene.addObject(cubeObject, 1);

  wg.setScene(scene);

  WingineCamera cam(F_PI/4, 9.0f/16.0f, 0.1f, 100.0f);
  Vector3 camPos(-5, 3, -10);
  cam.setLookAt(camPos,
		Vector3(0, 0, 0),
		Vector3(0, 1, 0));
  Matrix4 rotation = Matrix4(cosf(0.01f), sinf(0.01f), 0.0f, 0.0f,
			     -sinf(0.01f), cosf(0.01f), 0.0f, 0.0f,
			     0.0f, 0.0f, 1.0f, 0.0f,
			     0.0f, 0.0f, 0.0f, 1.0f);
  Matrix4 offset = Matrix4(1.0f, .0f, .0f, 1.0f,
			   .0f, 1.0f, .0f, .0f,
			   .0f, .0f, 1.0f, .5f,
			   .0f, .0f, .0f, 1.0f);


  clock_t start_time = clock();
  int count = 0;

  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.01f*count));
    offset = offset * rotation;
    Matrix4 newOffset = ~((~cam.getRenderMatrix())*offset);
    count++;
    Matrix4 cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.setUniform(offsetUniform, &newOffset, sizeof(Matrix4));
    wg.renderScene();
    clock_t current_time = clock();
    long long int diff = current_time - start_time;
    long long w = 1000/60 - 1000*diff/CLOCKS_PER_SEC;
    if(w > 0){
      win.sleepMilliseconds((int32_t)w);
    }
    start_time = current_time;
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  wg.destroyTexture(texture);
  wg.destroyUniform(cameraUniform);
  wg.destroyUniform(offsetUniform);

  wg.destroyImage(im);

  wg.destroyShader(vertexShader);
  wg.destroyShader(fragmentShader);
  wg.destroyShader(textureVertexShader);
  wg.destroyShader(textureFragmentShader);

  wg.destroyResourceSet(cameraSet);
  wg.destroyResourceSet(textureSet);
  wg.destroyResourceSet(kernelResources);

  wg.destroyKernel(kernel);

  wg.destroyResourceSetLayout(computeLayout);
  wg.destroyResourceSetLayout(resourceLayout);
  wg.destroyResourceSetLayout(textureResourceLayout);

  wg.destroyBuffer(vertexBuffer);
  wg.destroyBuffer(colorBuffer);
  wg.destroyBuffer(indexBuffer);
  wg.destroyBuffer(cubeVertexBuffer);
  wg.destroyBuffer(cubeIndexBuffer);
  wg.destroyBuffer(cubeTextureCoordBuffer);
  wg.destroyBuffer(textureCoordBuffer);

  return 0;
}
