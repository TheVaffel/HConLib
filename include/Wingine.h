//Vulkan integration for Winval

//Huge thanks to LunarG's Vulkan tutorial, from which most of the code is stolen - https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html

#ifndef INCLUDED_WINGINE
#define INCLUDED_WINGINE

#include <Winval.h>

#ifdef WINGINE_WITH_GLSLANG
#include <external/vulkan/SPIRV/GlslangToSpv.h>
#endif // WINGINE_WITH_GLSLANG

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

#define DEBUG

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define DEPTH_BUFFER_FORMAT VK_FORMAT_D32_SFLOAT

#define NUM_DESCRIPTOR_SETS 1

#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

#define MAX_COLOR_ATTACHMENTS 4
#define MAX_VERTEX_ATTRIBUTES 10
#define MAX_DESCRIPTOR_SETS 3
#define UNIFORM_DESCRIPTOR_POOL_SIZE 50
#define TEXTURE_DESCRIPTOR_POOL_SIZE 10
#define IMAGE_STORE_DESCRIPTOR_POOL_SIZE 10
#define MAX_NUM_COMMANDS 50

typedef enum WgShaderStage{
  WG_SHADER_STAGE_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
  WG_SHADER_STAGE_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
  WG_SHADER_STAGE_COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT
} WgShaderStage;

typedef enum WgResourceType{
  WG_RESOURCE_TYPE_TEXTURE = 0,
  WG_RESOURCE_TYPE_UNIFORM = 1,
  WG_RESOURCE_TYPE_STORE_IMAGE = 2
} WgResourceType;

#define NUM_WG_ATTACHMENT_TYPES 2
typedef enum WgAttachmentType {
  WG_ATTACHMENT_TYPE_COLOR = 0x0,
  WG_ATTACHMENT_TYPE_DEPTH = 0x1
} WgAttachmentType;

typedef enum WgAttribFormat {
  WG_ATTRIB_FORMAT_1 = 1,
  WG_ATTRIB_FORMAT_2 = 2,
  WG_ATTRIB_FORMAT_3 = 3,
  WG_ATTRIB_FORMAT_4 = 4
} WgAttribFormat;

#define WG_NUM_ATTRIB_TYPES 4
  typedef enum WgAttribType {
    WG_ATTRIB_TYPE_POSITION = 0x0,
    WG_ATTRIB_TYPE_COLOR = 0x1,
    WG_ATTRIB_TYPE_NORMAL = 0x2,
    WG_ATTRIB_TYPE_TEXTURE = 0x3
  } WgAttriblType;

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

class Wingine;

struct WingineBuffer{
  VkBuffer buffer;
  VkDeviceMemory memory;
};

struct WingineResource {
  virtual const VkDescriptorType getDescriptorType() const = 0;

  virtual const VkDescriptorImageInfo* getImageInfo() const {
    return NULL;
  }

  virtual const VkDescriptorBufferInfo* getBufferInfo() const {
    return NULL;
  }
};

struct WingineUniform : WingineResource{
  WingineBuffer buffer;
  VkDescriptorBufferInfo bufferInfo;

  virtual const VkDescriptorType getDescriptorType() const {
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  }

  virtual const VkDescriptorBufferInfo* getBufferInfo() const {
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
  int memorySize;

  virtual const VkDescriptorType getDescriptorType() const {
    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  }

  virtual const VkDescriptorImageInfo* getImageInfo() const {
    return &imageInfo;
  }
};

struct WingineTexture : WingineResource {
  WingineImage image;
  VkSampler sampler;

  virtual const VkDescriptorType getDescriptorType() const {
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  }

  virtual const VkDescriptorImageInfo* getImageInfo() const {
    return &image.imageInfo;
  }
};

struct WingineFramebuffer {
  WingineImage image;
  WingineImage depth_image;
  VkFramebuffer framebuffer;
};

struct WingineResourceSetLayout{
  VkDescriptorSetLayout layout;
  uint32_t numResources;
  WgResourceType *types;
};

struct WingineResourceSet{
  VkDescriptorSet descriptorSet;
  WingineResourceSetLayout* layout;
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
  WingineRenderPassSetup(int numAttachments, const WgAttachmentType* types);
  WingineRenderPassSetup(std::initializer_list<WgAttachmentType> types);
  ~WingineRenderPassSetup();

  VkAttachmentDescription* attachments;
  VkAttachmentReference* references;
  VkSubpassDescription subpass;
  VkRenderPassCreateInfo createInfo;
  int numAttachments;
  WgAttachmentType* attachmentTypes;
};

struct WinginePipeline{
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  int numVertexAttribs;
  VkRenderPass compatibleRenderPass;
  int numAttachments;
  int numDescriptorSetLayouts;
  VkDescriptorSetLayout* descriptorSetLayouts;
  bool clearAttachments;
};

struct WinginePipelineSetup{
  WinginePipelineSetup(int numAttachments, const WgAttachmentType* types);
  WinginePipelineSetup(std::initializer_list<WgAttachmentType> types);
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
 protected:
  std::vector<WingineBuffer> vertexAttribs;
  WingineBuffer indexBuffer;
  bool altered = true;
  const WinginePipeline* pipeline;
  WingineObjectGroup* objectGroup;

  int numDrawIndices;
  int indexOffset;
 public:
  WingineRenderObject();
  WingineRenderObject(int numInds, std::initializer_list<WingineBuffer> vertexBuffers, const WingineBuffer& indexBuffer);
  WingineRenderObject(int numInds, int numVertexAttribs, const WingineBuffer* buffers, const WingineBuffer& indexBuffer);

  void setPipeline(const WinginePipeline& p);
  void setIndexOffset(int newIndex);
  void setNumIndices(int num);
  uint32_t getNumIndices();
  uint32_t getIndexOffset();
  const WingineBuffer& getIndexBuffer();

  void setCommandBuffer(const VkCommandBuffer& cmd);
  VkCommandBuffer* getCommandBufferPointer();

  void setIndexBuffer(const WingineBuffer& indexBuffer);
  uint32_t getNumAttribs();
  const WingineBuffer* getAttribs();
  void setVertexAttribs(const WingineBuffer& wb, int index);

  void setObjectGroup(WingineObjectGroup& wog);
  bool isAltered();
  void destroy();
};

struct WingineObjectGroup{ // Collection of objects that are rendered with the same pipeline
private:
  Wingine* wingine;
public:
  WingineObjectGroup(Wingine& wg, WinginePipeline& pipeline);
  bool altered  = true;
  bool shouldClearAttachments;
  WinginePipeline pipeline;
  std::vector<WingineRenderObject> objects;
  VkCommandBuffer commandBuffer;

  void startRecording();
  void startRecording(const WingineFramebuffer& framebuffer);
  void endRecording();

  void addObject(const WingineRenderObject& obj);

  void recordRendering(WingineRenderObject& object,
		      std::initializer_list<WingineResourceSet> sets);
  void recordRendering(WingineRenderObject& object, const WingineResourceSet* sets);
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
  void setLookDirection(float rightAngle, float upAngle,
			const Vector3& up);
  
  Matrix4 getRenderMatrix();
  Matrix4 getTransformMatrix();
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

  VkSurfaceKHR surface;
  VkFormat format;
  VkQueueFamilyProperties* queue_props;
  VkSwapchainKHR swapchain;
  uint32_t swapchain_image_count;
  VkImage*swapchain_images;

  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue compute_queue;

  uint32_t width;
  uint32_t height;

  VkDescriptorPool descriptor_pool;
  VkDescriptorSet* descriptor_set;

  VkRenderPass render_pass_generic;

  WingineFramebuffer* framebuffers;

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

  VkResult init_instance(int width, int height, const char* title);
  VkResult find_device();
  VkResult init_device();
  VkResult init_device_queue();

#ifdef WIN32
  VkResult init_surface(HINSTANCE hinst, HWND hwnd);
#else // WIN32
  VkResult init_surface(Window window, Display* display);
#endif // WIN32

  VkResult init_command_buffers();
  VkResult init_swapchain();
  VkResult init_descriptor_pool();
  VkResult init_render_passes();
  VkResult init_framebuffers();
  VkResult init_pipeline_cache();

  void destroy_instance();
  void destroy_device();
  void destroy_command_buffers();
  void destroy_swapchain();
  void destroy_render_passes();
  void destroy_descriptor_pool();
  void destroy_framebuffers();
  void destroy_pipeline_cache();

  VkResult init_global_extension_properties();
  VkResult init_global_layer_properties();

  uint32_t get_memory_type_index(uint32_t, VkFlags);
  uint32_t get_format_element_size(WgAttribFormat format);
  VkFormat get_vkformat(WgAttribFormat att);
  VkDescriptorType get_descriptor_type(WgResourceType type);

  void destroy_vulkan();

  void printError(VkResult res);

  void pipeline_setup_generic(WinginePipelineSetup* setup, int numVertexAttribs);
  void pipeline_setup_color_renderer( WinginePipelineSetup* setup);
  void create_pipeline_color_renderer_single_buffer(VkPipeline*);
  void create_pipeline_color_renderer(VkPipeline*);

  void render_pass_setup_generic(WingineRenderPassSetup* setup);

  void image_create_info_generic(VkImageCreateInfo* ici);

  void wg_cmd_set_image_layout(VkCommandBuffer, VkImage, VkImageAspectFlags, VkImageLayout, VkImageLayout);

  void render_generic(VkPipeline, const WingineBuffer&, const WingineBuffer&, const WingineBuffer&, const Matrix4& model, bool shouldClear = false);
  void stage_next_image();

  void pushNewDrawSemaphore();

#if WIN32

  void initVulkan(int width, int height, const char* title,
		  HINSTANCE hinstance, HWND hwnd);
#else
  void initVulkan(int width, int height, const char* title,
             Window window, Display* display);
#endif

  WingineFramebuffer create_framebuffer_from_vk_image(VkImage image, uint32_t width, uint32_t height, bool hasMemory);
  WingineImage create_image_from_vk_image(VkImage vkim, uint32_t w, uint32_t h, VkImageUsageFlags usage, uint32_t memProps);
  WingineImage create_mappable_image(uint32_t width, uint32_t height);

 public:

  void copyImage(int w, int h, VkImage srcImage, VkImageLayout srcStartLayout, VkImageLayout srcEndLayout,
		 int w2, int h2, VkImage dstImage, VkImageLayout dstStartLayout, VkImageLayout dstEndLayout,
		 VkImageAspectFlagBits aspect);
  void copyColorImage(WingineImage& src, WingineImage& dst);
  void copyDepthImage(WingineImage& src, WingineImage& dst);
  void copyColorFromFramebuffer(WingineFramebuffer src, WingineImage dst);
  void copyDepthFromFramebuffer(WingineFramebuffer src, WingineImage dst);

  int getScreenWidth() const;
  int getScreenHeight() const;

  WingineFramebuffer getCurrentFramebuffer() const;
  WingineFramebuffer getLastFramebuffer() const;

  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level);
  VkRenderPass createDefaultRenderPass(); // One color attachment, one depth attachment

  WingineBuffer createBuffer(uint32_t, uint32_t, const void* data = NULL);
  WingineBuffer createVertexBuffer(uint32_t, const void* data = NULL);
  WingineBuffer createIndexBuffer(uint32_t, const void* data = NULL);
  VkResult setBuffer(const WingineBuffer&, const void*, uint32_t);
  void destroyBuffer(const WingineBuffer&);

  WingineImage createImage(uint32_t w, uint32_t h, VkImageLayout, VkImageUsageFlags,
			   VkImageTiling tiling=VK_IMAGE_TILING_OPTIMAL,
			   uint32_t memProps=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // Image is optimal-tiled by default and is not visible from host
  WingineImage createGPUImage(uint32_t w, uint32_t h);

  void setLayout(WingineImage& wim, VkImageLayout newLayout);
  void destroyImage(WingineImage);
  void getImageContent(WingineImage image, uint8_t* dst);
  WingineImage createDepthBuffer(uint32_t width, uint32_t height, uint32_t usage);
  WingineImage createDepthBuffer(uint32_t width, uint32_t height);

  WingineFramebuffer createFramebuffer(uint32_t width, uint32_t height);
  void destroyFramebuffer(WingineFramebuffer framebuffer);

  WingineImage createReadableDepthBuffer(uint32_t width, uint32_t height);
  WingineFramebuffer createDepthFramebuffer(uint32_t width, uint32_t height,
    WinginePipeline& pipeline);

  //A single uniform. Basically just a lump of data to be accessed from shaders
  WingineUniform createUniform(uint32_t size);
  void setUniform(const WingineUniform&, void*, uint32_t);
  void destroyUniform(const WingineUniform&);

  //A layout for resource sets
  // First evaluates stages for uniforms, then textures. Number of elements in stages = numUniforms + numTextures
  WingineResourceSetLayout createResourceSetLayout(std::initializer_list<WgResourceType> types,
						   std::initializer_list<WgShaderStage> stages);
  WingineResourceSetLayout createResourceSetLayout(std::initializer_list<WgResourceType> types,
						   std::initializer_list<VkShaderStageFlagBits> stages);
  WingineResourceSetLayout createResourceSetLayout(int numResources, const WgResourceType* types, const VkShaderStageFlagBits* stages);
  void destroyResourceSetLayout(WingineResourceSetLayout wrsl);

  //A set of uniforms and textures (to better utilize Vulkan's descriptor set abstraction) that "belong together"
  WingineResourceSet createResourceSet(WingineResourceSetLayout& layout);

  void updateResourceSet(WingineResourceSet& set, std::initializer_list<WingineResource const *> resources);
  // Second type is a bit mystical, but oh well
  void updateResourceSet(WingineResourceSet& set, const WingineResource* const* resources);
  void updateResourceSetIndex(WingineResourceSet& set, WingineResource* resource, int index); 
  void destroyResourceSet(const WingineResourceSet& resourceSet);

#ifdef WINGINE_WITH_GLSLANG
  WingineShader createShader(const char* shaderText, WgShaderStage stageBit);
  WingineShader createVertexShader(const char* shaderText);
  WingineShader createFragmentShader(const char* shaderText);
#endif // WINGINE_WITH_GLSLANG
  
  WingineShader createShader(const std::vector<uint32_t>& spirv, WgShaderStage stageBit);
  
  void destroyShader(WingineShader shader);

  WinginePipeline createPipeline(std::initializer_list<WingineResourceSetLayout> resourceLayouts,
				 std::initializer_list<WingineShader> shaders,
				 std::initializer_list<WgAttribFormat> attribFormats,
				 bool clear, std::initializer_list<WgAttachmentType> attachmentTypes);
  WinginePipeline createPipeline(int numResourceLayouts, const WingineResourceSetLayout* resourceLayouts,
    int numShaders, const WingineShader* shaders, int numVertexAttribs, const WgAttribFormat* attribTypes,
    bool clear, int numAttachments, const WgAttachmentType* attachmentTypes);
  WinginePipeline createPipeline(WingineResourceSetLayout resourceLayout,
    int numShaders, const WingineShader* shaders, int numVertexAttribs, const WgAttribFormat* attribTypes,
    bool clear, int numAttachments, const WgAttachmentType* attachmentTypes);
  WinginePipeline createPipeline(WingineResourceSetLayout layout, int numShaders,
    const WingineShader* shaders, int numVertexAttribs, const WgAttribFormat* attribTypes, bool clear = false);
  WinginePipeline createPipeline(std::initializer_list<WingineResourceSetLayout> layouts,
				 std::initializer_list<WingineShader> shaders,
				 std::initializer_list<WgAttribFormat> attribTypes,
				 bool clear);
  WinginePipeline createPipeline(int numResourceLayouts, const WingineResourceSetLayout* resourceLayouts,
				 std::initializer_list<WingineShader> shaders,
				 std::initializer_list<WgAttribFormat> attribFormats,
				 bool clear);
  WinginePipeline createDepthPipeline(std::initializer_list<WingineResourceSetLayout> layouts,
				      std::initializer_list<WingineShader> shaders);
  WinginePipeline createDepthPipeline(WingineResourceSetLayout layout,
				      int numShaders,
				      const WingineShader* shaders);
  WinginePipeline createDepthPipeline(int numLayouts, const WingineResourceSetLayout* layouts,
				      int numShaders,
				      const WingineShader* shaders);
  void destroyPipeline(WinginePipeline pipeline);

  WingineTexture createDepthTexture(int w, int h);
  WingineTexture createTexture(int w, int h, unsigned char* data);
  void destroyTexture(WingineTexture&);

  void destroyObject(WingineRenderObject&);

#ifdef WINGINE_WITH_GLSLANG
  WingineKernel createKernel(const char* kernelText, WingineResourceSetLayout layout);
#endif // WINGINE_WITH_GLSLANG
  
  void executeKernel(WingineKernel& kernel, WingineResourceSet resourceSet, int numX, int numY, int numZ);
  void destroyKernel(WingineKernel kernel);

  void present();

  void submitDrawCommandBuffer(const VkCommandBuffer& buffer);

  void setCamera(WingineCamera& camera);

#ifdef WIN32
  Wingine(int width, int height, const char* title, HINSTANCE hinst, HWND hwnd);
#endif // WIN32
  Wingine(const Winval&);

  ~Wingine();
};


namespace wgutil {
  
  enum WgModelInitMode {
    WG_MODEL_INIT_POLY,
    WG_MODEL_INIT_CUBE,
    WG_MODEL_INIT_SPHERE,
    WG_MODEL_INIT_QUAD,
    WG_MODEL_INIT_READ_OBJ
  };
  
  struct Model : public WingineRenderObject {
  protected:
    Wingine* wingine;
    Matrix4 transform;
    WingineUniform transformUniform;
    WingineResourceSet resourceSet;

    void initModel(Wingine& wg);
    
    void initPolyhedron(std::initializer_list<uint32_t> sizes,
			std::initializer_list<void (*)(float, float, float*)> generators,
			int numT, int numH, float t10, float t11);
    void initFromFile(const char* file_name, std::initializer_list<WgAttribType> attribs);
    void initCube(std::initializer_list<WgAttribType> attribs, float size);
    void initQuad(std::initializer_list<WgAttribType> attribs,
		  const Vector3& side1, const Vector3& side2);
  public:
    // Create from .obj file
    Model(Wingine& w, WgModelInitMode mode, const char* filename, std::initializer_list<WgAttribType> attribs);

    // Create from generator functions
    Model(Wingine& w, WgModelInitMode mode, std::initializer_list<uint32_t> sizes,
	  std::initializer_list<void (*)(float, float, float*)> generators,
	  int numT, int numH, float t10 = 1.0f, float t11 = 1.0f);

    // Basic geometric shapes (cube, sphere...)
    Model(Wingine& w, WgModelInitMode mode,
	  std::initializer_list<WgAttribType> attribs, float size);
    // Quad
    Model(Wingine& w, WgModelInitMode mode,
	  std::initializer_list<WgAttribType> attribs,
	  const Vector3& side1, const Vector3& side2);
    
    // Legacy, remove when ColorModel is removed and initCube is implemented
    Model(Wingine& w);
    void destroy();
    
    WingineResourceSet& getTransformSet();
    void setTransform(const Matrix4& mat);
  };
};

typedef WingineBuffer WgBuffer;
typedef WingineImage WgImage;
typedef WingineShader WgShader;
typedef WingineKernel WgKernel;
typedef WingineCamera WgCamera;
typedef WingineUniform WgUniform;
typedef WingineTexture WgTexture;
typedef WingineResourceSetLayout WgRSL;
typedef WingineResource WgResource;
typedef WingineResourceSet WgRS;
typedef WingineResourceSet WgResourceSet;
typedef WingineRenderObject WgRenderObject;
typedef WingineRenderObject WgObject;
typedef WinginePipeline WgPipeline;
typedef WingineObjectGroup WgOG;
typedef WingineObjectGroup WgObjectGroup;
typedef WingineFramebuffer WgFramebuffer;

#define GLSL(src) "#version  450\n#extension GL_ARB_separate_shader_objects : enable\n#extension GL_ARB_shading_language_420pack : enable\n"#src

#endif //INCLUDED_WINGINE
