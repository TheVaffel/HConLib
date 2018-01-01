#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <Winval.h>
#include <Wingine.h>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <external/stb_image_write.h>

using namespace std;

static const float test_vertices[] =
  { 0.0f, 0.0f, 0.5f, 1.0f,
    1.0f, 0.0f, 0.5f, 1.0f,
    0.0f, 1.0f, 0.5f, 1.0f,
    1.0f, 1.0f, 0.5f, 1.0f};

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
  "  imageStore(outputs, coords, vec4(coords.x/1920.0, coords.y/1080.0, 0.0, 1));\n"
  "}";


int main(){
  const int texWidth = 1000;
  const int texHeight = 500;
  unsigned char* generic_pattern = new unsigned char[4*texWidth*texHeight];
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
    cube_tex_coords[2*i + 1] = pop & 2 ? 1.0f : 0.0f;
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

  WgBuffer vertexBuffer = wg.createVertexBuffer(4*4*sizeof(float), test_vertices);

  WingineBuffer colorBuffer = wg.createVertexBuffer(4*4*sizeof(float), test_colors);

  WingineBuffer indexBuffer = wg.createIndexBuffer(3*2*sizeof(int32_t), test_indices);

  WingineBuffer textureCoordBuffer = wg.createVertexBuffer(4*2*sizeof(float), texture_coords);

  WingineBuffer cubeVertexBuffer = wg.createVertexBuffer(8*4*sizeof(float), cube_vertices);

  WingineBuffer cubeIndexBuffer = wg.createIndexBuffer(12*3*sizeof(int), cube_indices);

  WingineBuffer cubeTextureCoordBuffer = wg.createVertexBuffer(8*2*sizeof(float), cube_tex_coords);

  VkShaderStageFlagBits bits[1] = {VK_SHADER_STAGE_VERTEX_BIT};
  VkShaderStageFlagBits textureResourceSetStageBits[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  WingineUniform cameraUniform = wg.createUniform(sizeof(Matrix4));
  WingineTexture texture = wg.createTexture(texWidth, texHeight, generic_pattern);
  WingineUniform offsetUniform = wg.createUniform(sizeof(Matrix4));

  WgResourceType desc[] = {WG_RESOURCE_TYPE_UNIFORM};
  WingineResourceSetLayout resourceLayout = wg.createResourceSetLayout(1, desc, bits);

  WgResourceType textDescriptorTypes[] = {WG_RESOURCE_TYPE_UNIFORM, WG_RESOURCE_TYPE_TEXTURE};
  WingineResourceSetLayout textureResourceLayout = wg.createResourceSetLayout(2, textDescriptorTypes, textureResourceSetStageBits);

  WingineResource* cameraUniformResource = &cameraUniform;
  WingineResourceSet cameraSet = wg.createResourceSet(resourceLayout, &cameraUniformResource);
  WingineResource* resources[] = {&offsetUniform, &texture};
  WingineResourceSet textureSet = wg.createResourceSet(textureResourceLayout, resources);

  WingineBuffer vertexAttribs[2] = {vertexBuffer, colorBuffer};
  WingineBuffer textureVertexAttribs[2] = {vertexBuffer, textureCoordBuffer};

  WgAttribFormat attribTypes[] = {WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_4};
  WgAttribFormat attribTypesTexture[] = {WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_2};

  WingineShader vertexShader = wg.createVertexShader(vertShaderText);
  WingineShader fragmentShader = wg.createFragmentShader(fragShaderText);

  WingineShader textureVertexShader = wg.createVertexShader(texVertShaderText);
  WingineShader textureFragmentShader = wg.createFragmentShader(texFragShaderText);

  WingineShader shaders[2] = {vertexShader, fragmentShader};
  WingineShader textureShaders[2] = {textureVertexShader, textureFragmentShader};

  WingineScene scene(wg);
  scene.addPipeline(resourceLayout, 2, shaders, 2, attribTypes); //pipelineLayout, num shaders, shaders, num vertex attribs, vertex attrib types
  scene.addPipeline(textureResourceLayout, 2, textureShaders, 2, attribTypesTexture);
  WingineRenderObject object1(6, 2, vertexAttribs, indexBuffer, cameraSet); //Num drawIndices, num vertex attribs, vertex attribs, index buffer, resourceset
  WingineRenderObject object3(6, 2, textureVertexAttribs, indexBuffer, textureSet);

  WingineBuffer cubeVertexAttribs[2] = {cubeVertexBuffer, cubeTextureCoordBuffer};
  WingineRenderObject cubeObject(3*12, 2, cubeVertexAttribs, cubeIndexBuffer, textureSet);

  wgutil::ColorModel cube1 = wgutil::createCube(wg, 0.45f);

  scene.addObject(object1, 0); //Object, pipeline number
  scene.addObject(object3, 1);
  scene.addObject(cube1, 0);
  
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

  // For demo purposes:
  /*uint8_t* image_data = new uint8_t[4 * 64*64];
  wg.getImageContent(im, image_data);

  stbi_write_jpg("texture_image.png", 64, 64, 4, image_data, 64*4);

  delete[] image_data;*/

  //WingineFramebuffer framebuffer = wg.createFramebuffer(64, 64);

  // #onlycomputethings
  desc[0] = WG_RESOURCE_TYPE_STORE_IMAGE;
  bits[0] = VK_SHADER_STAGE_COMPUTE_BIT;

  WingineResourceSetLayout computeLayout = wg.createResourceSetLayout(1, desc, bits);
  WingineKernel kernel = wg.createKernel(computeShaderText, computeLayout);

  WingineImage im = wg.createGPUImage(texWidth, texHeight);
  wg.setLayout(im, VK_IMAGE_LAYOUT_GENERAL);
  WingineResource* computeImageResource[] = {&im};
  WingineResourceSet kernelResources = wg.createResourceSet(computeLayout,computeImageResource);
  wg.executeKernel(kernel, kernelResources, texWidth, texHeight, 1);
  wg.copyImage(im, texture.image);

  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.01f*count));
    offset = offset * rotation;
    Matrix4 newOffset = ~((~cam.getRenderMatrix())*offset);
    count++;
    Matrix4 cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.setUniform(offsetUniform, &newOffset, sizeof(Matrix4));
    wg.setUniform(*(WingineUniform*)cube1.getResource(0), &cMatrix, sizeof(Matrix4));
    wg.renderScene();

    //WingineFramebuffer framebuffer = wg.getLastFramebuffer();
    //wg.copyFromFramebuffer(framebuffer, texture.image);

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
