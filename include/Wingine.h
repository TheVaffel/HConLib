//Vulkan integration for Winval

//Huge thanks to LunarG's Vulkan tutorial, from which most of this is stolen - https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html

#ifndef INCLUDED_WINGINE
#define INCLUDED_WINGINE

#include <Winval.h>
#include <external/vulkan/SPIRV/GlslangToSpv.h>
#include <unistd.h>

#include <FlatAlg.h>

#define VK_USE_PLATFORM_XLIB_KHR
#include "external/vulkan/vulkan.h"

#include <stdio.h> // printf
#include <vector> //vector
#include <cstdlib> // exit
#include <algorithm> //min, max

#include <ctime>
#include <unistd.h>

#define DEBUG

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

#define NUM_DESCRIPTOR_SETS 1

#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

#define MAX_COLOR_ATTACHMENTS 4
#define MAX_VERTEX_ATTRIBUTES 10
#define DESCRIPTOR_POOL_SIZE 10

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

struct WingineBuffer{
  VkBuffer buffer;
  VkDeviceMemory memory;
};

struct WingineUniform{
  WingineBuffer buffer;
  VkDescriptorBufferInfo bufferInfo;
};

struct WingineUniformSet{
  VkDescriptorSet descriptorSet;
  VkDescriptorSetLayout descriptorSetLayout;
  const char* name;
};

struct WingineShader{
  VkPipelineShaderStageCreateInfo shader;
  WingineUniformSet* uniformSets;
  int numUniformSets;
};

struct WinginePipeline{
  VkPipeline pipeline;
  VkDescriptorSet* descriptorSets;
  int numDescriptorSets;
  VkPipelineLayout pipelineLayout;
  int numVertexAttribs;
};

struct WinginePipelineSetup{
  VkGraphicsPipelineCreateInfo createInfo;
  VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
  VkPipelineDynamicStateCreateInfo dynamicState;
  VkVertexInputBindingDescription* vi_bindings;
  VkVertexInputAttributeDescription* vi_attribs;
  VkPipelineVertexInputStateCreateInfo vi;
  VkPipelineInputAssemblyStateCreateInfo ia;
  VkPipelineRasterizationStateCreateInfo rs;
  VkPipelineColorBlendStateCreateInfo cb;
  VkPipelineColorBlendAttachmentState att_state[MAX_COLOR_ATTACHMENTS];
  VkPipelineViewportStateCreateInfo vp;
  VkPipelineDepthStencilStateCreateInfo ds;
  VkPipelineMultisampleStateCreateInfo ms;
  int numShaders;
};

class WingineCamera{
  const Matrix4 clip = {1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 0.5f, 0.5f,
			0.f, 0.f, 0.0f, 1.f};
  Matrix4 projection, view, total;
  bool altered;
 public:
  
  WingineCamera(float horizontalFOVRadians = 45.f/180.f*M_PI, float invAspect = 9.0f/16.0f, float near = 0.1f, float far = 100.0f);

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
  
  uint current_buffer;
  uint queue_family_count;
  uint graphics_queue_family_index;
  uint present_queue_family_index;
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSurfaceKHR surface;
  VkFormat format;
  VkQueueFamilyProperties* queue_props;
  VkSwapchainKHR swapchain;
  uint swapchain_image_count;
  VkImage*swapchain_images;
  VkImageView* swapchain_image_views;

  VkQueue graphics_queue;
  VkQueue present_queue;

  VkFormat depth_buffer_format;
  VkImage depth_buffer_image;
  VkDeviceMemory depth_buffer_memory;
  VkImageView depth_buffer_view;
  
  uint width;
  uint height;

  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_memory;
  VkDescriptorBufferInfo uniform_buffer_info;
  
  VkDescriptorSetLayout* desc_layout;
  VkPipelineLayout pipeline_layout;

  VkDescriptorPool descriptor_pool;
  VkDescriptorSet* descriptor_set;

  VkRenderPass render_pass;

  VkFramebuffer* framebuffers;

  VkPipeline color_pipeline;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineCache pipeline_cache;
  
  WingineBuffer VPCUniform;
  WingineBuffer ModelTransformUniform;
  
  std::vector<VkLayerProperties> instance_layer_properties;
  std::vector<const char*> instance_extension_names;
  std::vector<const char*> instance_layer_names;
  std::vector<const char*> device_extension_names;

  VkPipelineShaderStageCreateInfo shaderStages[2];

  VkVertexInputBindingDescription vi_binding;
  VkVertexInputAttributeDescription vi_attribs[2];
  
  VkResult init_instance(const Winval*);
  VkResult find_device();
  VkResult init_device();
  VkResult init_device_queue();
  VkResult init_surface(const Winval*);
  VkResult init_command_buffers();
  VkResult init_swapchain();
  VkResult init_depth_buffer();
  void init_uniform_buffer();
  VkResult init_descriptor_set_layouts();
  VkResult init_descriptor_pool();
  VkResult init_descriptor_set();
  VkResult init_render_pass();
  VkResult init_shaders();
  VkResult init_framebuffers();
  VkResult init_pipeline_cache();
  VkPipeline init_pipeline();

  void destroy_instance();
  void destroy_device();
  void destroy_command_buffers();
  void destroy_swapchain();
  void destroy_depth_buffer();
  void destroy_uniform_buffer();
  void destroy_descriptor_set_layouts();
  void destroy_descriptor_set();
  void destroy_render_pass();
  void destroy_shaders();
  void destroy_framebuffers();
  void destroy_pipeline_cache();
  void destroy_pipeline();

  VkResult init_global_extension_properties();
  VkResult init_global_layer_properties();

  uint get_memory_type_index(uint, VkFlags);

  void destroy_vulkan();

  void printError(VkResult res);

  void pipeline_setup_generic(WinginePipelineSetup* setup, int numVertexAttribs);
  void pipeline_setup_color_renderer( WinginePipelineSetup* setup);
  void create_pipeline_color_renderer_single_buffer(VkPipeline*);
  void create_pipeline_color_renderer(VkPipeline*);
  
  void updateMVP(const Matrix4&);
  void render_generic(VkPipeline, const WingineBuffer&, const WingineBuffer&, const WingineBuffer&, const Matrix4& model);

 public:

  WingineBuffer createBuffer(uint, uint);
  VkResult setBuffer(const WingineBuffer&, const void*, uint);
  void destroyBuffer(const WingineBuffer&);

  //A single uniform. Basically just a lump of data to be accessed from shaders
  WingineUniform createUniform(uint size);
  void setUniform(const WingineUniform&, void*, uint);
  void destroyUniform(const WingineUniform&);
  
  //A set of uniforms (to better utilize Vulkan's descriptor set abstraction) that "belong together"
  WingineUniformSet createUniformSet(int numUniforms, WingineUniform* uniforms, VkShaderStageFlagBits* shaderStages, const char* name);
  void destroyUniformSet(const WingineUniformSet& uniformSet);

  WingineShader createShader(const char* shaderText, int numUniformSets, const WingineUniformSet* uniformSets, VkShaderStageFlagBits stageBit);
  void destroyShader(WingineShader shader);

  WinginePipeline createPipeline(int numShaders, WingineShader* shaders, int numVertexAttribs);
  void destroyPipeline(WinginePipeline pipeline);
  
  void renderColor(const WingineBuffer&, const WingineBuffer&, const WingineBuffer&, const Matrix4& model);
  void render(const WingineBuffer* vertexAttribs, const WingineBuffer& indices, const WinginePipeline& pipeline, bool clear);
  void setCamera(WingineCamera& camera);
  void initVulkan(const Winval*);

  Wingine(const Winval&);

  ~Wingine();
};

class ColorModel{
  uint32_t numVertices;
  float* vertices;
  float* colors;
  uint32_t* indices;

  WingineBuffer vertexBuffer;

  WingineBuffer colorBuffer;

  WingineBuffer indexBuffer;

 public:
  ColorModel(uint32_t nVerts, uint32_t nInds);
  ~ColorModel();
};

#endif //INCLUDED_WINGINE
