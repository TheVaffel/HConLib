//Vulkan integration for Winval

//Huge thanks to LunarG's Vulkan tutorial, from which most of this is stolen - https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html

#ifndef INCLUDED_WINGINE
#define INCLUDED_WINGINE

#include <Winval.h>
#include <external/vulkan/SPIRV/GlslangToSpv.h>

#include <FlatAlg.h>

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else //WIN32
#define VK_USE_PLATFORM_XLIB_KHR
#endif //WIN32

#include "external/vulkan/vulkan.h"

#include <stdio.h> // printf
#include <vector> //vector

#include <ctime>
//#include <unistd.h>

#define DEBUG

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

#define NUM_DESCRIPTOR_SETS 1

#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

#define MAX_COLOR_ATTACHMENTS 4
#define MAX_VERTEX_ATTRIBUTES 10
#define UNIFORM_DESCRIPTOR_POOL_SIZE 50
#define TEXTURE_DESCRIPTOR_POOL_SIZE 10
#define IMAGE_STORE_DESCRIPTOR_POOL_SIZE 10
#define MAX_NUM_COMMANDS 100

#define WINGINE_RESOURCE_TEXTURE VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
#define WINGINE_RESOURCE_UNIFORM VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
#define WINGINE_RESOURCE_STORE_IMAGE VK_DESCRIPTOR_TYPE_STORAGE_IMAGE

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

class Wingine;

struct WingineBuffer{
  VkBuffer buffer;
  VkDeviceMemory memory;
};

struct WingineResource {
  virtual VkDescriptorType getDescriptorType() = 0;
  
  virtual VkDescriptorImageInfo* getImageInfo(){
    return NULL;
  }
  
  virtual VkDescriptorBufferInfo* getBufferInfo(){
    return NULL;
  }
};

struct WingineUniform : WingineResource{
  WingineBuffer buffer;
  VkDescriptorBufferInfo bufferInfo;

  virtual VkDescriptorType getDescriptorType(){
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  }

  virtual VkDescriptorBufferInfo* getBufferInfo(){
    return &bufferInfo;
  }
};


struct WingineImage : WingineResource {
  uint32_t width, height;
  VkImage image;
  VkImageLayout layout;
  VkDeviceMemory mem;
  VkImageView view;
  VkDescriptorImageInfo imageInfo;

  virtual VkDescriptorType getDescriptorType(){
    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  }

  virtual VkDescriptorImageInfo* getImageInfo(){
    return &imageInfo;
  }
};

struct WingineTexture : WingineResource {
  WingineImage image;
  VkSampler sampler;

  virtual VkDescriptorType getDescriptorType(){
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  }

  virtual VkDescriptorImageInfo* getImageInfo(){
    return &image.imageInfo;
  }
};


struct WingineResourceSetLayout{
  VkDescriptorSetLayout layout;
  int numResources;
};

struct WingineResourceSet{
  VkDescriptorSet descriptorSet;
};

struct WingineShader{
  VkPipelineShaderStageCreateInfo shader;
  int numUniformSets;
};

struct WingineKernel{
  WingineShader shader;
  VkPipeline pipeline;
  VkPipelineLayout layout;
};

struct WingineRenderPassSetup{
  WingineRenderPassSetup(int numAttachments = 2);
  ~WingineRenderPassSetup();

  VkAttachmentDescription* attachments;
  VkAttachmentReference* references;
  VkSubpassDescription subpass;
  VkRenderPassCreateInfo createInfo;
  int numAttachments;
};

struct WinginePipeline{
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  int numVertexAttribs;
  VkRenderPass compatibleRenderPass;
  int numAttachments;
  VkDescriptorSetLayout descriptorSetLayout;
};

struct WinginePipelineSetup{
  WinginePipelineSetup(int numColorAttachments = 1);
  ~WinginePipelineSetup();
  
  VkGraphicsPipelineCreateInfo createInfo;
  VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
  VkPipelineDynamicStateCreateInfo dynamicState;
  VkVertexInputBindingDescription* vi_bindings;
  VkVertexInputAttributeDescription* vi_attribs;
  VkPipelineVertexInputStateCreateInfo vi;
  VkPipelineInputAssemblyStateCreateInfo ia;
  VkPipelineRasterizationStateCreateInfo rs;
  VkPipelineColorBlendStateCreateInfo cb;
  VkPipelineColorBlendAttachmentState* att_state;
  VkPipelineViewportStateCreateInfo vp;
  VkPipelineDepthStencilStateCreateInfo ds;
  VkPipelineMultisampleStateCreateInfo ms;
  VkPipelineLayoutCreateInfo layoutCreateInfo;
  int numShaders;
  WingineRenderPassSetup renderPassSetup;
};

struct WingineObjectGroup;

class WingineRenderObject{
  std::vector<WingineBuffer> vertexAttribs;
  WingineBuffer indexBuffer;
  VkCommandBuffer commandBuffer;
  WingineResourceSet uniformSet;
  bool altered = true;
  const WinginePipeline* pipeline;
  WingineObjectGroup* objectGroup;

  int numDrawIndices;
  int indexOffset;
  
 public:
  WingineRenderObject(int numInds, int numVertexAttribs, WingineBuffer* buffers, const WingineBuffer& indexBuffer, const WingineResourceSet& rSet);
  
  void setPipeline(const WinginePipeline& p);
  void setIndexOffset(int newIndex);

  void recordCommandBuffer(VkCommandBuffer& cmd);
  void setCommandBuffer(const VkCommandBuffer& cmd);
  VkCommandBuffer* getCommandBufferPointer();
  
  void setIndexBuffer(const WingineBuffer& indexBuffer);
  void setVertexAttribs(const WingineBuffer& wb, int index);

  void setObjectGroup(WingineObjectGroup& wog);
  bool isAltered();
  
};

struct WingineObjectGroup{ // Collection of all objects (in a scene) that are rendered with the same pipeline
private:
  const Wingine* wingine;
public:
  WingineObjectGroup(const Wingine& wg);
  bool altered  = true;
  bool shouldClearAttachments;
  WinginePipeline pipeline;
  std::vector<WingineRenderObject> objects;
  VkCommandBuffer commandBuffer;

  void recordCommandBuffer();
  
  void startRecordingCommandBuffer();
  void endRecordingCommandBuffer();
};

class WingineScene{
  Wingine* wg;
public:
  std::vector<WingineObjectGroup> objectGroups;
  WingineScene(Wingine& wg);
  ~WingineScene();
  void addPipeline(WingineResourceSetLayout layout, int numShaders, WingineShader* shaders, int numVertexAttribs, VkFormat* attribTypes);
  void addObject(const WingineRenderObject& obj, int pipelineInd);
  
};

class WingineCamera{
  const Matrix4 clip = {1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 0.5f, 0.5f,
			0.f, 0.f, 0.0f, 1.f};
  Matrix4 projection, view, total;
  bool altered;
 public:

  WingineCamera(float horizontalFOVRadians = 45.f/180.f*F_PI, float invAspect = 9.0f/16.0f, float near = 0.1f, float far = 100.0f);

  void setPosition(const Vector3& v);

  void setLookAt(const Vector3& pos,
		 const Vector3& target,
		 const Vector3& up);

  Matrix4 getRenderMatrix();

};

class Wingine{

  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties device_props;
  VkPhysicalDeviceMemoryProperties device_memory_props;
  VkDevice device;

  uint32_t current_buffer;
  uint32_t queue_family_count;
  uint32_t graphics_queue_family_index;
  uint32_t present_queue_family_index;
  uint32_t compute_queue_family_index;
  
  VkCommandPool cmd_pool;
  VkCommandPool compute_cmd_pool;
  
  VkCommandBuffer free_command_buffer;
  VkCommandBuffer compute_command_buffer;
  VkFence free_command_buffer_fence;
  VkFence compute_command_buffer_fence;
  //VkCommandBuffer cmd_buffers[MAX_NUM_COMMANDS];
  //uint32_t current_command_buffer;
  
  VkSurfaceKHR surface;
  VkFormat format;
  VkQueueFamilyProperties* queue_props;
  VkSwapchainKHR swapchain;
  uint32_t swapchain_image_count;
  VkImage*swapchain_images;
  VkImageView* swapchain_image_views;

  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue compute_queue;

  VkFormat depth_buffer_format;
  VkImage depth_buffer_image;
  VkDeviceMemory depth_buffer_memory;
  VkImageView depth_buffer_view;

  uint32_t width;
  uint32_t height;
  
  VkDescriptorPool descriptor_pool;
  VkDescriptorSet* descriptor_set;

  VkRenderPass render_pass_generic;

  VkFramebuffer* framebuffers;

  std::vector<VkSemaphore> drawSemaphores;
  uint32_t currSemaphore;

  VkFence imageAcquiredFence;

  VkPipeline color_pipeline;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineCache pipeline_cache;
  
#ifdef DEBUG
  VkDebugReportCallbackEXT debugCallback;
#endif //DEBUG
  
  std::vector<VkLayerProperties> instance_layer_properties;
  std::vector<const char*> instance_extension_names;
  std::vector<const char*> instance_layer_names;
  std::vector<const char*> device_extension_names;

  WingineScene* currentScene;
  
  VkResult init_instance(const Winval*);
  VkResult find_device();
  VkResult init_device();
  VkResult init_device_queue();
  VkResult init_surface(const Winval*);
  VkResult init_command_buffers();
  VkResult init_swapchain();
  VkResult init_depth_buffer();
  VkResult init_descriptor_pool();
  VkResult init_render_passes();
  VkResult init_framebuffers();
  VkResult init_pipeline_cache();
  VkPipeline init_pipeline();

  void destroy_instance();
  void destroy_device();
  void destroy_command_buffers();
  void destroy_swapchain();
  void destroy_depth_buffer();
  void destroy_render_passes();
  void destroy_descriptor_pool();
  void destroy_framebuffers();
  void destroy_pipeline_cache();
  void destroy_pipeline();

  VkResult init_global_extension_properties();
  VkResult init_global_layer_properties();

  uint32_t get_memory_type_index(uint32_t, VkFlags);

  void destroy_vulkan();

  void printError(VkResult res);

  void pipeline_setup_generic(WinginePipelineSetup* setup, int numVertexAttribs);
  void pipeline_setup_color_renderer( WinginePipelineSetup* setup);
  void create_pipeline_color_renderer_single_buffer(VkPipeline*);
  void create_pipeline_color_renderer(VkPipeline*);

  void render_pass_setup_generic(WingineRenderPassSetup* setup);

  void wg_cmd_set_image_layout(VkCommandBuffer, VkImage, VkImageAspectFlags, VkImageLayout, VkImageLayout, VkPipelineStageFlags, VkPipelineStageFlags);
  
  void render_generic(VkPipeline, const WingineBuffer&, const WingineBuffer&, const WingineBuffer&, const Matrix4& model, bool shouldClear = false);
  void stage_next_image();

  void pushNewDrawSemaphore();

  
 public:
  
  void copyImage(int w, int h, VkImage srcImage, VkImageLayout srcStartLayout, VkImageLayout srcEndLayout, VkImage dstImage, VkImageLayout dstStartLayout, VkImageLayout dstEndLayout);
  
  int getScreenWidth() const;
  int getScreenHeight() const;

  VkFramebuffer getCurrentFramebuffer() const;

  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level);
  VkRenderPass createDefaultRenderPass(); // One color attachment, one depth attachment

  WingineBuffer createBuffer(uint32_t, uint32_t);
  VkResult setBuffer(const WingineBuffer&, const void*, uint32_t);
  void destroyBuffer(const WingineBuffer&);

  WingineImage createImage(uint32_t w, uint32_t h, VkImageLayout, VkImageUsageFlags);
  void setLayout(WingineImage& wim, VkImageLayout newLayout);
  void destroyImage(WingineImage);

  //A single uniform. Basically just a lump of data to be accessed from shaders
  WingineUniform createUniform(uint32_t size);
  void setUniform(const WingineUniform&, void*, uint32_t);
  void destroyUniform(const WingineUniform&);

  //A layout for resource sets
  // First evaluates stages for uniforms, then textures. Number of elements in stages = numUniforms + numTextures
  WingineResourceSetLayout createResourceSetLayout(int numResources, VkDescriptorType* types, VkShaderStageFlagBits* stages);
  void destroyResourceSetLayout(WingineResourceSetLayout wrsl);
  
  //A set of uniforms and textures (to better utilize Vulkan's descriptor set abstraction) that "belong together"
  WingineResourceSet createResourceSet(WingineResourceSetLayout layout, WingineResource **resources);
  void destroyResourceSet(const WingineResourceSet& resourceSet);

  WingineShader createShader(const char* shaderText, VkShaderStageFlagBits stageBit);
  void destroyShader(WingineShader shader);

  WinginePipeline createPipeline(WingineResourceSetLayout layout, int numShaders, WingineShader* shaders, int numVertexAttribs, VkFormat* attribTypes, bool clear = false);
  void destroyPipeline(WinginePipeline pipeline);

  WingineTexture createTexture(int w, int h, unsigned char* data);
  void destroyTexture(WingineTexture&);

  WingineKernel createKernel(const char* kernelText, WingineResourceSetLayout layout);
  void executeKernel(WingineKernel& kernel, WingineResourceSet resourceSet, int numX, int numY, int numZ);
  void destroyKernel(WingineKernel kernel);
  
  void renderColor(const WingineBuffer&, const WingineBuffer&, const WingineBuffer&, const Matrix4& model);
  void render(const WingineBuffer* vertexAttribs, const WingineBuffer& indices, const WinginePipeline& pipeline, bool clear);

  void synchronizeDrawing();
  void present();

  void submitDrawCommandBuffer(const VkCommandBuffer& buffer);

  void setCamera(WingineCamera& camera);

  void setScene(WingineScene& scene);
  void renderScene();
  
  void initVulkan(const Winval*);

  Wingine(const Winval&);

  ~Wingine();
};

#endif //INCLUDED_WINGINE
