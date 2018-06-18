#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <Winval.h>

#define WINGINE_WITH_GLSLANG
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
  GLSL(
       layout (std140, binding = 0) uniform bufferVals {
	 mat4 mvp;
       } myBufferVals;
       layout (location = 0) in vec4 pos;
       layout (location = 1) in vec4 inColor;
       layout (location = 0) out vec4 outColor;
       out gl_PerVertex { 
	 vec4 gl_Position;
       };
       void main() {
	 outColor = inColor;
	 gl_Position = myBufferVals.mvp * pos;
       }
       );

const char *fragShaderText =
  GLSL(
       layout (location = 0) in vec4 color;
       layout (location = 0) out vec4 outColor;
       void main() {
	 outColor = color;
       }
       );

const char *texVertShaderText =
  GLSL(
       layout (std140, binding = 0) uniform bufferVals {
	 mat4 mvp;
       } myBufferVals;
       layout (location = 0) in vec4 pos;
       layout (location = 1) in vec2 texCoord;
       layout (location = 0) out vec2 outTexCoord;
       out gl_PerVertex { 
	 vec4 gl_Position;
       };
       void main() {
	 outTexCoord = texCoord;
	 gl_Position = myBufferVals.mvp * pos;
       }
       );

const char* texFragShaderText =
  GLSL(
       layout (binding = 1) uniform sampler2D tex;
       layout (location = 0) in vec2 texCoord;
       layout (location = 0) out vec4 outColor;
       void main() {
	 outColor = textureLod(tex, texCoord, 0.0);
       }
       );

const char* computeShaderText =
  GLSL(
       layout(binding = 0, rgba32f) uniform image2D outputs;
       layout (local_size_x = 16, local_size_y = 16) in;
       void main() {
	 ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	 imageStore(outputs, coords, vec4(coords.x/1920.0, coords.y/1080.0, 0.0, 1));
       }
       );


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

  WingineUniform cameraUniform = wg.createUniform(sizeof(Matrix4));
  WingineTexture texture = wg.createTexture(texWidth, texHeight, generic_pattern);
  WingineUniform offsetUniform = wg.createUniform(sizeof(Matrix4));

  WingineResourceSetLayout resourceLayout =
    wg.createResourceSetLayout({WG_RESOURCE_TYPE_UNIFORM},
			       {VK_SHADER_STAGE_VERTEX_BIT});

  WingineResourceSetLayout textureResourceLayout =
    wg.createResourceSetLayout({WG_RESOURCE_TYPE_UNIFORM, WG_RESOURCE_TYPE_TEXTURE},
			       {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT});

  WingineResourceSet cameraSet = wg.createResourceSet(resourceLayout, {&cameraUniform});
  WingineResourceSet textureSet = wg.createResourceSet(textureResourceLayout, {&offsetUniform, &texture});

  WingineShader vertexShader = wg.createVertexShader(vertShaderText);
  WingineShader fragmentShader = wg.createFragmentShader(fragShaderText);

  WingineShader textureVertexShader = wg.createVertexShader(texVertShaderText);
  WingineShader textureFragmentShader = wg.createFragmentShader(texFragShaderText);

  WinginePipeline colorPipeline = wg.createPipeline({resourceLayout},
						    {vertexShader, fragmentShader},
						    {WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_4}, true);
  WingineObjectGroup colorGroup(wg, colorPipeline);
  WinginePipeline texturePipeline = wg.createPipeline({textureResourceLayout},
						      {textureVertexShader, textureFragmentShader},
						      {WG_ATTRIB_FORMAT_4, WG_ATTRIB_FORMAT_2}, false);
  WingineObjectGroup textureGroup(wg, texturePipeline);

  WingineRenderObject object1(6, {vertexBuffer, colorBuffer}, indexBuffer);
  WingineRenderObject object3(6, {vertexBuffer, textureCoordBuffer}, indexBuffer);

  WingineRenderObject cubeObject(3*12, {cubeVertexBuffer, cubeTextureCoordBuffer}, cubeIndexBuffer);
  
  wgutil::ColorModel cube1 = wgutil::createCube(wg, 0.45f);


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
  WingineResourceSetLayout computeLayout = wg.createResourceSetLayout({WG_RESOURCE_TYPE_STORE_IMAGE}, {VK_SHADER_STAGE_COMPUTE_BIT});
  WingineKernel kernel = wg.createKernel(computeShaderText, computeLayout);

  WingineImage im = wg.createGPUImage(texWidth, texHeight);
  wg.setLayout(im, VK_IMAGE_LAYOUT_GENERAL);
  
  WingineResourceSet kernelResources = wg.createResourceSet(computeLayout,{&im});
  wg.executeKernel(kernel, kernelResources, texWidth, texHeight, 1);
  wg.copyColorImage(im, texture.image);

  while(win.isOpen()){
    cam.setPosition(camPos + 0.5f*camPos*sin(0.01f*count));
    offset = offset * rotation;
    Matrix4 newOffset = ~((~cam.getRenderMatrix())*offset);
    count++;
    Matrix4 cMatrix = cam.getRenderMatrix();
    wg.setUniform(cameraUniform, &cMatrix, sizeof(Matrix4));
    wg.setUniform(offsetUniform, &newOffset, sizeof(Matrix4));
 
    colorGroup.startRecording();
    colorGroup.recordRendering(object1, {cameraSet});
    colorGroup.recordRendering(cube1, {cameraSet});
    colorGroup.endRecording();

    textureGroup.startRecording();
    textureGroup.recordRendering(object3, {textureSet});
    textureGroup.recordRendering(cubeObject, {textureSet});
    textureGroup.endRecording();

    wg.present();
    
    // For inspiration ;)
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

  cube1.destroy();

  wg.destroyTexture(texture);
  wg.destroyUniform(cameraUniform);
  wg.destroyUniform(offsetUniform);

  wg.destroyImage(im);

  wg.destroyShader(vertexShader);
  wg.destroyShader(fragmentShader);
  wg.destroyShader(textureVertexShader);
  wg.destroyShader(textureFragmentShader);

  wg.destroyPipeline(colorPipeline);
  wg.destroyPipeline(texturePipeline);

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
