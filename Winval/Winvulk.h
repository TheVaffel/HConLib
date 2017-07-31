//Vulkan integration for Winval

//Huge amount is stolen from LunarG's Vulkan tutorial - https://vulkan.lunarg.com/doc/sdk/latest/linux/getting_started.html

#ifndef INCLUDED_WINVULK
#define INCLUDED_WINVULK

#include <unistd.h>

#define VK_USE_PLATFORM_XCB_KHR
#include "vulkan/vulkan.h"

#include <stdio.h> // printf
#include <vector> //vector
#include <cstdlib> // exit
#include <algorithm> //min, max

#include "vulkan/glsl_util.h"
#include "vulkan/cube_data.h"


#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define DEBUG

struct winvulk_vulkan_state {
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

  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkDescriptorBufferInfo vertex_buffer_info;

  VkPipeline pipeline;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineCache pipeline_cache;
  
  std::vector<VkLayerProperties> instance_layer_properties;
  std::vector<const char*> instance_extension_names;
  std::vector<const char*> instance_layer_names;
  std::vector<const char*> device_extension_names;

  VkPipelineShaderStageCreateInfo shaderStages[2];

  VkVertexInputBindingDescription vi_binding;
  VkVertexInputAttributeDescription vi_attribs[2];
};

VkResult init_global_extension_properties();
VkResult init_global_layer_properties();

void winvulk_init_vulkan(winvulk_vulkan_state*, Winval*);

VkResult winvulk_init_instance(winvulk_vulkan_state*, Winval*);
VkResult winvulk_find_device(winvulk_vulkan_state*);
VkResult winvulk_init_device(winvulk_vulkan_state*);
VkResult winvulk_init_surface(winvulk_vulkan_state*, Winval*);
VkResult winvulk_init_command_buffers(winvulk_vulkan_state*);
VkResult winvulk_init_swapchain(winvulk_vulkan_state*);
VkResult winvulk_init_depth_buffer(winvulk_vulkan_state*);
VkResult winvulk_init_uniform_buffer(winvulk_vulkan_state*);
VkResult winvulk_init_descriptor_set_layouts(winvulk_vulkan_state*);
VkResult winvulk_init_descriptor_pool(winvulk_vulkan_state*);
VkResult winvulk_init_descriptor_set(winvulk_vulkan_state*);
VkResult winvulk_init_render_pass(winvulk_vulkan_state*);
VkResult winvulk_init_shaders(winvulk_vulkan_state*);
VkResult winvulk_init_framebuffers(winvulk_vulkan_state*);
VkResult winvulk_init_vertex_buffer(winvulk_vulkan_state*);
VkResult winvulk_init_pipeline_cache(winvulk_vulkan_state*);
VkResult winvulk_init_pipeline(winvulk_vulkan_state*);

void winvulk_destroy_instance(winvulk_vulkan_state*);
void winvulk_destroy_device(winvulk_vulkan_state*);
void winvulk_destroy_command_buffers(winvulk_vulkan_state*);
void winvulk_destroy_swapchain(winvulk_vulkan_state*);
void winvulk_destroy_depth_buffer(winvulk_vulkan_state*);
void winvulk_destroy_uniform_buffer(winvulk_vulkan_state*);
void winvulk_destroy_descriptor_set_layouts(winvulk_vulkan_state*);
void winvulk_destroy_descriptor_set(winvulk_vulkan_state*);
void winvulk_destroy_render_pass(winvulk_vulkan_state*);
void winvulk_destroy_shaders(winvulk_vulkan_state*);
void winvulk_destroy_framebuffers(winvulk_vulkan_state*);
void winvulk_destroy_vertex_buffer(winvulk_vulkan_state*);
void winvulk_destroy_pipeline_cache(winvulk_vulkan_state*);
void winvulk_destroy_pipeline(winvulk_vulkan_state*);

void winvulk_destroy_vulkan(winvulk_vulkan_state*);

void winvulk_render(winvulk_vulkan_state*);

#endif //INCLUDED_WINVULK

#ifdef WINVAL_VULKAN_IMPLEMENTATION

#define FLATALG_IMPLEMENTATION
#include <FlatAlg/FlatAlg.h>

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

#define NUM_DESCRIPTOR_SETS 1

#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

void printError(VkResult res){
  if(res == VK_ERROR_OUT_OF_HOST_MEMORY){
    printf("Host memory\n");
  }else if(res == VK_ERROR_OUT_OF_DEVICE_MEMORY){
    printf("Device memory\n");
  }else if(res == VK_ERROR_DEVICE_LOST){
    printf("Device lost\n");
  }else if(res == VK_ERROR_SURFACE_LOST_KHR){
    printf("Surface lost KHR\n");
  }else if(res == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR){
    printf("Window is already in use");
  }else if(res == VK_ERROR_INITIALIZATION_FAILED){
    printf("Initialization failed\n");
  }else if(res == VK_ERROR_EXTENSION_NOT_PRESENT){
    printf("Required extension not present\n");
  }else if(res == VK_ERROR_FEATURE_NOT_PRESENT){
    printf("Feature not present\n");
  }else{
    printf("Error not identified\n");
  }
}


#define GET_INSTANCE_PROC_ADDR(instance, entry)			\
  (PFN_vk##entry)vkGetInstanceProcAddr(instance, "vk" #entry);

#define GET_DEVICE_PROC_ADDR(device, entry)		\
  (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry);

uint winvulk_get_memory_type_index(winvulk_vulkan_state* vs, uint type_bits, VkFlags requirements_mask){
  for(int i = 0; i < vs->device_memory_props.memoryTypeCount; i++){
    if((type_bits & 1) == 1){
      if((vs->device_memory_props.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask){
	return i;
      }
    }
    type_bits >>= 1;
  }

  //If we got here, appropriate memory type not found
  printf("Appropriate memory type could not be found\n");
  exit(0);
}

VkResult winvulk_init_instance(winvulk_vulkan_state* vs, Winval* win){
  vs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  vs->instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

#ifdef DEBUG
  vs->instance_extension_names.push_back("VK_EXT_debug_report");
  vs->instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
  
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = win->getTitle();
  app_info.applicationVersion = 1;
  app_info.pEngineName = "Winvulk";
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledExtensionCount = vs->instance_extension_names.size();
  inst_info.ppEnabledExtensionNames = vs->instance_extension_names.data();
  inst_info.enabledLayerCount = vs->instance_layer_names.size();
  inst_info.ppEnabledLayerNames = vs->instance_layer_names.size()? vs->instance_layer_names.data(): NULL;

  VkResult res;

  res = vkCreateInstance(&inst_info, NULL, &vs->instance);

  if( res == VK_ERROR_INCOMPATIBLE_DRIVER){
    printf("Cannot find a compatible Vulkan ICD\n");
    exit(-1);
  }
  
  vs->width = win->getWidth();
  vs->height = win->getHeight();
}

VkResult winvulk_find_device(winvulk_vulkan_state* vs){
  unsigned int device_count;
  
  VkResult res;
  vkEnumeratePhysicalDevices(vs->instance, &device_count, NULL);
  VkPhysicalDevice* pDevs = new VkPhysicalDevice[device_count];
  res = vkEnumeratePhysicalDevices(vs->instance, &device_count, pDevs);
  printf("Number of devices: %d\n", device_count);

    
  bool foundDevice = 0;
  
  for(int i = 0; i < device_count; i++){
    vkGetPhysicalDeviceProperties(pDevs[i], &vs->device_props);
    vkGetPhysicalDeviceMemoryProperties(pDevs[i], &vs->device_memory_props);
    printf("Device %i: %s\n", i, vs->device_props.deviceName);
    
    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &vs->queue_family_count, NULL);
    vs->queue_props = new VkQueueFamilyProperties[vs->queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &vs->queue_family_count, vs->queue_props);

    for(int j = 0; j < vs->queue_family_count; j++){
      if(vs->queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
        vs->physical_device = pDevs[i];
	foundDevice = 1;
	break;
      }
    }

    if(foundDevice)
      break;
  }

  if(!foundDevice){
    printf("Did not find fit device\n");
    exit(0);
  }

  return res;
}

VkResult winvulk_init_device(winvulk_vulkan_state* vs){

  VkBool32 *pSupportsPresent = new VkBool32[vs->queue_family_count];
  for(int i = 0 ; i < vs->queue_family_count; i++){
    vkGetPhysicalDeviceSurfaceSupportKHR(vs->physical_device, i, vs->surface, &pSupportsPresent[i]);
  }

  vs->graphics_queue_family_index = UINT32_MAX;
  vs->present_queue_family_index = UINT32_MAX;
  
  for(int i = 0; i < vs->queue_family_count; i++){
    if((vs->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      if(vs->graphics_queue_family_index == UINT32_MAX) vs->graphics_queue_family_index = i;

      if(pSupportsPresent[i] == VK_TRUE){
	vs->present_queue_family_index = i;
	vs->graphics_queue_family_index = i;
	break;
      }
    }
  }

  if(vs ->present_queue_family_index == UINT32_MAX){
    for(int i = 0; i < vs->queue_family_count; i++){
      if(pSupportsPresent[i] == VK_TRUE){
	vs->present_queue_family_index = i;
	break;
      }
    }
  }

  if(vs->present_queue_family_index == UINT32_MAX || vs->graphics_queue_family_index == UINT32_MAX){
    printf("Failed to find present- and/or graphics queue\n");
    exit(0);
  }

  delete[] pSupportsPresent;
  
  
  vs->device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  
  VkDeviceQueueCreateInfo queue_info = {};
  float queue_priorities[1] = {0.0f};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;
  queue_info.queueFamilyIndex = vs->graphics_queue_family_index;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = vs->device_extension_names.size();
  device_info.ppEnabledExtensionNames = vs->device_extension_names.data();
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = NULL;
  device_info.pEnabledFeatures = NULL;

  VkDevice device;

  VkResult res = vkCreateDevice(vs->physical_device, &device_info, NULL, &device);
  
  vs->device = device;

  if(res != VK_SUCCESS){
    printf("Device creation failed \n");
  }

  return res;
}

VkResult winvulk_init_command_buffers(winvulk_vulkan_state* vs){
  VkCommandPoolCreateInfo cmd_pool_info = {};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.pNext = NULL;
  cmd_pool_info.queueFamilyIndex = vs->graphics_queue_family_index;
  cmd_pool_info.flags = 0;

  VkResult res = vkCreateCommandPool(vs->device, &cmd_pool_info, NULL, &vs->cmd_pool);
  if(res != VK_SUCCESS){
    printf("Failed to create command buffer pool \n");
  }
  
  VkCommandBufferAllocateInfo cmd_alloc = {};
  cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_alloc.pNext = NULL;
  cmd_alloc.commandPool = vs->cmd_pool;
  cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_alloc.commandBufferCount = 1;

  res = vkAllocateCommandBuffers(vs->device, &cmd_alloc, &vs->cmd_buffer);
  
  if(res != VK_SUCCESS){
    printf("Failed to allocate command buffer\n");
    exit(0);
  }

  return res;
}

VkResult winvulk_init_surface(winvulk_vulkan_state* vs, Winval* win){
  VkXcbSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.window = win->getWindow();
  surface_info.connection = win->getConnection();

  VkResult res = vkCreateXcbSurfaceKHR(vs->instance, &surface_info, NULL, &vs->surface);
  if(res != VK_SUCCESS){
    printf("Could not create surface\n");
    exit(0);
  }
}

VkResult winvulk_init_device_queue(winvulk_vulkan_state* vs){
  vkGetDeviceQueue(vs->device, vs->graphics_queue_family_index, 0, &vs->graphics_queue);
  if(vs->graphics_queue_family_index == vs->present_queue_family_index){
    vs->present_queue = vs->graphics_queue;
  }else{
    vkGetDeviceQueue(vs->device, vs->present_queue_family_index, 0, &vs->present_queue);
  }

  return VK_SUCCESS;
}

VkResult winvulk_init_swapchain(winvulk_vulkan_state* vs){

  uint formatCount;
  VkResult res = vkGetPhysicalDeviceSurfaceFormatsKHR(vs->physical_device, vs->surface, &formatCount, NULL);
  if(res != VK_SUCCESS){
    printf("Could not get format count\n");
    exit(0);
  }
  
  VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(vs->physical_device, vs->surface, &formatCount, formats);

  if(res != VK_SUCCESS){
    printf("Could not get formats\n");
    exit(0);
  }

  if(formatCount == 1 &&  formats[0].format == VK_FORMAT_UNDEFINED){
    vs->format = VK_FORMAT_B8G8R8A8_UNORM;
  } else if (formatCount > 1){
    vs->format = formats[0].format;
  }else{
    printf("Did not get any format\n");
    exit(0);
  }

  delete[] formats;

  VkSurfaceCapabilitiesKHR caps;

  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vs->physical_device, vs->surface, &caps);
  if(res != VK_SUCCESS){
    printf("Could not get surface capabilities\n");
    exit(0);
  }

  uint presentModeCount;
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(vs->physical_device, vs->surface, &presentModeCount, NULL);
  if(res != VK_SUCCESS){
    printf("Could not get present mode count\n");
  }
  
  VkPresentModeKHR*presentModes = new VkPresentModeKHR[presentModeCount];
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(vs->physical_device, vs->surface, &presentModeCount, presentModes);
  if(res != VK_SUCCESS){
    printf("Could not get present modes\n");
    exit(0);
  }

  VkExtent2D swapchainExtent;

  if(caps.currentExtent.width == 0xFFFFFFFF){

    swapchainExtent.width = vs->width;
    swapchainExtent.height = vs->height;

    swapchainExtent.width = std::max(vs->width, caps.minImageExtent.width);
    swapchainExtent.height = std::max(vs->height, caps.minImageExtent.height);

    swapchainExtent.width = std::min(vs->width, caps.maxImageExtent.width);
    swapchainExtent.height = std::min(vs->height, caps.maxImageExtent.height);

  }else{
    swapchainExtent = caps.currentExtent;
  }

  VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  uint numSwaps = caps.minImageCount;

  VkSurfaceTransformFlagBitsKHR preTransform;
  if(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR){
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }else{
    preTransform = caps.currentTransform;
  }

  VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
  };
  
  for(int i = 0; i < sizeof(compositeAlphaFlags); i++){
    if(caps.supportedCompositeAlpha & compositeAlphaFlags[i]){
      compositeAlpha = compositeAlphaFlags[i];
      break;
    }
  }

  uint queueFamilyIndices[2] = {vs->graphics_queue_family_index, vs->present_queue_family_index};

  VkSwapchainCreateInfoKHR swapchain_ci = {};
  swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_ci.pNext = NULL;
  swapchain_ci.surface = vs->surface;
  swapchain_ci.minImageCount = numSwaps;
  swapchain_ci.imageFormat = vs->format;
  swapchain_ci.imageExtent.width = swapchainExtent.width;
  swapchain_ci.imageExtent.height = swapchainExtent.height;
  swapchain_ci.preTransform = preTransform;
  swapchain_ci.compositeAlpha = compositeAlpha;
  swapchain_ci.imageArrayLayers = 1;
  swapchain_ci.presentMode = swapchainPresentMode;
  swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
  swapchain_ci.clipped = true;
  swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_ci.queueFamilyIndexCount = 0;
  swapchain_ci.pQueueFamilyIndices = NULL;
  
  if(vs->graphics_queue_family_index != vs->present_queue_family_index){
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
    printf("Got different family queues for graphics and presenting, setting concurrent mode in swapchain\n");
  }

  res = vkCreateSwapchainKHR(vs->device, &swapchain_ci, NULL, &vs->swapchain);

  if(res != VK_SUCCESS){
    printf("Failed to create swapchain\n");
    printError(res);
    exit(0);
  }

  res = vkGetSwapchainImagesKHR(vs->device, vs->swapchain, &vs->swapchain_image_count, NULL);

  VkImage* swapchainImages = new VkImage[vs->swapchain_image_count];

  res = vkGetSwapchainImagesKHR(vs->device, vs->swapchain, &vs->swapchain_image_count, swapchainImages);

  if(res != VK_SUCCESS){
    printf("Failed to get swapchain images\n");
    exit(0);
  }

  vs->swapchain_images = new VkImage[vs->swapchain_image_count];
  for(int i = 0; i < vs->swapchain_image_count; i++){
    vs->swapchain_images[i] = swapchainImages[i];
  }
  delete[] swapchainImages;

  vs->swapchain_image_views = new VkImageView[vs->swapchain_image_count];
  for(int i = 0; i < vs->swapchain_image_count; i++){
    VkImageViewCreateInfo color_image_view = {};
    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = NULL;
    color_image_view.flags = 0;
    color_image_view.image = vs->swapchain_images[i];
    color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_image_view.format = vs->format;
    color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
    color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
    color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
    color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
    color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_image_view.subresourceRange.baseMipLevel = 0;
    color_image_view.subresourceRange.levelCount = 1;
    color_image_view.subresourceRange.baseArrayLayer = 0;
    color_image_view.subresourceRange.layerCount = 1;

    res = vkCreateImageView(vs->device, &color_image_view, NULL, &vs->swapchain_image_views[i]);
    if(res != VK_SUCCESS){
      printf("Could not create image view for image %d\n", i);
    }
  }
 
}

VkResult winvulk_init_depth_buffer(winvulk_vulkan_state* vs){
  VkImageCreateInfo image_info = {};

  const VkFormat depth_format = VK_FORMAT_D16_UNORM;
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(vs->physical_device, depth_format, &props);
  if(props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
  }else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  }else{
    printf("Did not support 16-bit depth format\n");
    exit(0);
  }

  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = NULL;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = depth_format;
  image_info.extent.width = vs->width;
  image_info.extent.height = vs->height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = NUM_SAMPLES;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = NULL;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.flags = 0;

  VkMemoryAllocateInfo mem_alloc = {};
  mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_alloc.pNext = NULL;
  mem_alloc.allocationSize = 0;
  mem_alloc.memoryTypeIndex = 0;

  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = NULL;
  view_info.image = VK_NULL_HANDLE;
  view_info.format = depth_format;
  view_info.components.r = VK_COMPONENT_SWIZZLE_R;
  view_info.components.g = VK_COMPONENT_SWIZZLE_G;
  view_info.components.b = VK_COMPONENT_SWIZZLE_B;
  view_info.components.a = VK_COMPONENT_SWIZZLE_A;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.flags = 0;

  VkMemoryRequirements mem_reqs;
  vs->depth_buffer_format = depth_format;

  VkResult res = vkCreateImage(vs->device, &image_info, NULL, &vs->depth_buffer_image);
  if(res != VK_SUCCESS){
    printf("Could not create depth buffer\n");
    exit(0);
  }

  vkGetImageMemoryRequirements(vs->device, vs->depth_buffer_image, &mem_reqs);
  mem_alloc.allocationSize = mem_reqs.size;

  mem_alloc.memoryTypeIndex = winvulk_get_memory_type_index(vs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  res = vkAllocateMemory(vs->device, &mem_alloc, NULL, &vs->depth_buffer_memory);
  if(res != VK_SUCCESS){
    printf("Memory could not be allocated\n");
    exit(0);
  }

  res = vkBindImageMemory(vs->device, vs->depth_buffer_image, vs->depth_buffer_memory, 0);
  if(res != VK_SUCCESS){
    printf("Memory could not be bound\n");
    exit(0);
  }

  view_info.image = vs->depth_buffer_image;
  res = vkCreateImageView(vs->device, &view_info, NULL, &vs->depth_buffer_view);
  if(res != VK_SUCCESS){
    printf("Depth image view could not be created\n");
    exit(0);
  }

  return res;
  
}

VkResult winvulk_init_uniform_buffer(winvulk_vulkan_state* vs){
  /*Matrix4 projection = flatalg::projection(45.f/180.0f*M_PI, 9.0f/16.0f, 0.1f, 100.0f);

  Matrix4 view = Matrix4(1.f, 0.f, 0.f, 0.f,
			 0.f, 1.f, 0.f, 0.f,
			 .0f, .0f, 1.f, 2.f,
			 .0f, .0f, .0f, 1.0f);
  Matrix4 model = Matrix4(1.f, 0.f, 0.f, -1.f,
			  0.f, 1.f, 0.f, 1.f,
			  0.f, 0.f, 1.f, -15.f,
			  0.f, 0.f, 0.f, 1.f);
  Matrix4 clip = Matrix4(1.f, 0.f, 0.f, 0.f,
			 0.f, -1.f, 0.f, 0.f,
			 0.f, 0.f, 0.5f, 0.f,
			 0.f, 0.f, 0.5f, 1.f);*/
  float fov = glm::radians(45.0f);
  if (vs->width > vs->height) {
    fov *= static_cast<float>(vs->height) / static_cast<float>(vs->width);
  }
  glm::mat4 Projection = glm::perspective(fov, static_cast<float>(vs->width) / static_cast<float>(vs->height), 0.1f, 100.0f);
  glm::mat4 View = glm::lookAt(glm::vec3(-5, 3, -10),  // Camera is at (-5,3,-10), in World Space
			       glm::vec3(0, 0, 0),     // and looks at the origin
			       glm::vec3(0, -1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
			       );
  glm::mat4 Model = glm::mat4(1.0f);
  // Vulkan clip space has inverted Y and half Z.
  glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

  glm::mat4 mvp = Clip * Projection * View * Model;

  //Matrix4 mvp = clip*projection*view*model;
  //printf("MVP = %s\n", mvp.str().c_str());

  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  buf_info.size = sizeof(mvp);
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  VkResult res = vkCreateBuffer(vs->device, &buf_info, NULL, &vs->uniform_buffer);

  if(res != VK_SUCCESS){
    printf("Could not create uniform buffer\n");
    exit(0);
  }

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(vs->device, vs->uniform_buffer, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  alloc_info.memoryTypeIndex = winvulk_get_memory_type_index(vs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if(alloc_info.memoryTypeIndex < 0){
    printf("Could not find proper memory\n");
    exit(0);
  }

  res = vkAllocateMemory(vs->device, &alloc_info, NULL, &vs->uniform_memory);
  if(res != VK_SUCCESS){
    printf("Colud not allocate memory\n");
    exit(0);
  }

  unsigned char *pData;
  res = vkMapMemory(vs->device, vs->uniform_memory, 0, mem_reqs.size, 0, (void**)&pData);
  if(res != VK_SUCCESS){
    printf("Could not map memory\n");
    exit(0);
  }

  memcpy(pData, &mvp, sizeof(mvp));

  vkUnmapMemory(vs->device, vs->uniform_memory);

  res = vkBindBufferMemory(vs->device, vs->uniform_buffer, vs->uniform_memory, 0);
  if(res != VK_SUCCESS){
    printf("Could not bind buffer and memory\n");
  }

  vs->uniform_buffer_info.buffer = vs->uniform_buffer;
  vs->uniform_buffer_info.offset = 0;
  vs->uniform_buffer_info.range = sizeof(mvp);
}

VkResult winvulk_init_descriptor_set_layouts(winvulk_vulkan_state* vs){
  VkDescriptorSetLayoutBinding layout_binding = {};
  layout_binding.binding = 0;
  layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layout_binding.descriptorCount = 1;
  layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layout_binding.pImmutableSamplers = NULL;

  VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
  descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout.pNext = NULL;
  descriptor_layout.bindingCount = 1;
  descriptor_layout.pBindings = &layout_binding;

  vs->desc_layout = new VkDescriptorSetLayout[NUM_DESCRIPTOR_SETS];
  VkResult res = vkCreateDescriptorSetLayout(vs->device, &descriptor_layout, NULL, vs->desc_layout);
  if(res != VK_SUCCESS){
    printf("Could not create descriptor set layout\n");
    exit(0);
  }

  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pPipelineLayoutCreateInfo.pNext = NULL;
  pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
  pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
  pPipelineLayoutCreateInfo.pSetLayouts = vs->desc_layout;

  res = vkCreatePipelineLayout(vs->device, &pPipelineLayoutCreateInfo, NULL, &vs->pipeline_layout);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline layout\n");
    exit(0);
  }
}

VkResult winvulk_init_descriptor_pool(winvulk_vulkan_state* vs){
  VkDescriptorPoolSize type_count[1];
  type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count[0].descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptor_pool = {};
  descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool.pNext = NULL;
  descriptor_pool.maxSets = 1;
  descriptor_pool.poolSizeCount = 1;
  descriptor_pool.pPoolSizes = type_count;

  VkResult res = vkCreateDescriptorPool(vs->device, &descriptor_pool, NULL, &vs->descriptor_pool);
  if(res != VK_SUCCESS){
    printf("Could not create descriptor pool\n");
    exit(0);
  }
}

VkResult winvulk_init_descriptor_set(winvulk_vulkan_state * vs){
  VkDescriptorSetAllocateInfo alloc_info[1];
  alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info[0].pNext = NULL;
  alloc_info[0].descriptorPool = vs->descriptor_pool;
  alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
  alloc_info[0].pSetLayouts = vs->desc_layout;

  vs->descriptor_set = new VkDescriptorSet[NUM_DESCRIPTOR_SETS];
  VkResult res = vkAllocateDescriptorSets(vs->device, alloc_info, vs->descriptor_set);
  if(res != VK_SUCCESS){
    printf("Could not allocate descriptor sets\n");
    exit(0);
  }

  VkWriteDescriptorSet writes[1];
  writes[0] = {};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].pNext = NULL;
  writes[0].dstSet = vs->descriptor_set[0];
  writes[0].descriptorCount = 1;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes[0].pBufferInfo = &vs->uniform_buffer_info;
  writes[0].dstArrayElement = 0;
  writes[0].dstBinding = 0;
  vkUpdateDescriptorSets(vs->device, 1, writes, 0, NULL);
}

VkResult winvulk_init_render_pass(winvulk_vulkan_state* vs){
  VkAttachmentDescription attachments[2];
  attachments[0].format = vs->format;
  attachments[0].samples = NUM_SAMPLES;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].flags = 0;

  attachments[1].format = vs->depth_buffer_format;
  attachments[1].samples = NUM_SAMPLES;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[1].flags = 0;

  VkAttachmentReference color_reference = {};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_reference = {};
  depth_reference.attachment = 1;
  depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = &depth_reference;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = 2;
  rp_info.pAttachments = attachments;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 0;
  rp_info.pDependencies = NULL;

  VkResult res = vkCreateRenderPass(vs->device, &rp_info, NULL, &vs->render_pass);
  if(res != VK_SUCCESS){
    printf("Could not create render pass\n");
    exit(0);
  }
}

VkResult winvulk_init_shaders(winvulk_vulkan_state* vs){
  static const char *vertShaderText =
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

    static const char *fragShaderText =
        "#version 400\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "layout (location = 0) in vec4 color;\n"
        "layout (location = 0) out vec4 outColor;\n"
        "void main() {\n"
        "   outColor = color;\n"
        "}\n";

    std::vector<unsigned int> vtx_spv;
    vs->shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vs->shaderStages[0].pNext = NULL;
    vs->shaderStages[0].pSpecializationInfo = NULL;
    vs->shaderStages[0].flags = 0;
    vs->shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs->shaderStages[0].pName = "main";

    glslang::InitializeProcess();
    bool retVal = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
    if(!retVal){
      printf("Could not compile vertex shader\n");
      exit(0);
    }

    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = vtx_spv.data();
    VkResult res = vkCreateShaderModule(vs->device, &moduleCreateInfo, NULL, &vs->shaderStages[0].module);
    if(res != VK_SUCCESS){
      printf("Could not create vertex shadermodule\n");
      exit(0);
    }

    std::vector<unsigned int> frag_spv;
    vs->shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vs->shaderStages[1].pNext = NULL;
    vs->shaderStages[1].pSpecializationInfo = NULL;
    vs->shaderStages[1].flags = 0;
    vs->shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vs->shaderStages[1].pName = "main";

    retVal = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
    if(!retVal){
      printf("Could not compile fragment shader\n");
      exit(0);
    }

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = frag_spv.data();
    res = vkCreateShaderModule(vs->device, &moduleCreateInfo, NULL, &vs->shaderStages[1].module);
    if(res != VK_SUCCESS){
      printf("Could not create fragment shader module\n");
      exit(0);
    }

    glslang::FinalizeProcess();
}

VkResult winvulk_init_framebuffers(winvulk_vulkan_state* vs){
  VkImageView attachments[2];
  attachments[1] = vs->depth_buffer_view;
  
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = vs->render_pass;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = attachments;
  fb_info.width = vs->width;
  fb_info.height = vs->height;
  fb_info.layers = 1;

  vs->framebuffers = new VkFramebuffer[vs->swapchain_image_count];

  VkResult res;
  for(int i = 0 ; i < vs->swapchain_image_count; i++){
    attachments[0] = vs->swapchain_image_views[i];
    res = vkCreateFramebuffer(vs->device, &fb_info, NULL, &vs->framebuffers[i]);
    if(res != VK_SUCCESS){
      printf("Could create framebuffer number %d\n", i);
      exit(0);
    }
  }
}

VkResult winvulk_init_vertex_buffer(winvulk_vulkan_state* vs){
  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buf_info.size = sizeof(g_vb_solid_face_colors_Data);
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  VkResult res = vkCreateBuffer(vs->device, &buf_info, NULL, &vs->vertex_buffer);

  if(res != VK_SUCCESS){
    printf("Could not create vertex buffer\n");
    exit(0);
  }

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements( vs->device, vs->vertex_buffer, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  alloc_info.memoryTypeIndex = winvulk_get_memory_type_index(vs, mem_reqs.memoryTypeBits,
						    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if(alloc_info.memoryTypeIndex < 0){
    printf("Could not find memory type for vertex buffer\n");
    exit(0);
  }

  res = vkAllocateMemory(vs->device, &alloc_info, NULL, &vs->vertex_buffer_memory);
  if(res != VK_SUCCESS){
    printf("Could not allocate memory for vertex buffer\n");
    exit(0);
  }
  vs->vertex_buffer_info.range = mem_reqs.size;
  vs->vertex_buffer_info.offset = 0;

  unsigned char *pData;
  res = vkMapMemory(vs->device, vs->vertex_buffer_memory, 0, mem_reqs.size, 0, (void**)&pData);
  if(res != VK_SUCCESS){
    printf("Could not map vertex buffer memory\n");
  }

  memcpy(pData, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data));

  vkUnmapMemory(vs->device, vs->vertex_buffer_memory);

  res = vkBindBufferMemory(vs->device, vs->vertex_buffer, vs->vertex_buffer_memory, 0);
  if(res != VK_SUCCESS){
    printf("Could not bind vertex buffer to memory\n");
    exit(0);
  }

  //For pipeline creation:
  vs->vi_binding.binding = 0;
  vs->vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vs->vi_binding.stride = sizeof(g_vb_solid_face_colors_Data[0]);

  vs->vi_attribs[0].binding = 0;
  vs->vi_attribs[0].location = 0;
  vs->vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  vs->vi_attribs[0].offset = 0;
  vs->vi_attribs[1].binding = 0;
  vs->vi_attribs[1].location = 1;
  vs->vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  vs->vi_attribs[1].offset = 16;
  
}

VkResult winvulk_init_pipeline_cache(winvulk_vulkan_state* vs){
  VkPipelineCacheCreateInfo pipelineCache;
  pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCache.pNext = NULL;
  pipelineCache.initialDataSize = 0;
  pipelineCache.pInitialData = NULL;
  pipelineCache.flags = 0;
  VkResult res = vkCreatePipelineCache(vs->device, &pipelineCache, NULL, &vs->pipeline_cache);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline cache\n");
  }
}

VkResult winvulk_init_pipeline(winvulk_vulkan_state* vs){
  
  VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.pNext = NULL;
  dynamicState.pDynamicStates = dynamicStateEnables;
  dynamicState.dynamicStateCount = 0;

  VkPipelineVertexInputStateCreateInfo vi;
  vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vi.pNext = NULL;
  vi.flags = 0;
  vi.vertexBindingDescriptionCount = 1;
  vi.pVertexBindingDescriptions = &vs->vi_binding;
  vi.vertexAttributeDescriptionCount = 2;
  vi.pVertexAttributeDescriptions = vs->vi_attribs;

  VkPipelineInputAssemblyStateCreateInfo ia;
  ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia.pNext = NULL;
  ia.flags = 0;
  ia.primitiveRestartEnable = VK_FALSE;
  ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo rs;
  rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rs.pNext = NULL;
  rs.flags = 0;
  rs.polygonMode = VK_POLYGON_MODE_FILL;
  rs.cullMode = VK_CULL_MODE_BACK_BIT;
  rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rs.depthClampEnable = VK_FALSE;
  rs.rasterizerDiscardEnable = VK_FALSE;
  rs.depthBiasEnable = VK_FALSE;
  rs.depthBiasConstantFactor = 0;
  rs.depthBiasClamp = 0;
  rs.depthBiasSlopeFactor = 0;
  rs.lineWidth = 1.0f;

  VkPipelineColorBlendStateCreateInfo cb;
  cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cb.pNext = NULL;
  cb.flags = 0;

  VkPipelineColorBlendAttachmentState att_state[1];
  att_state[0].colorWriteMask = 0xf;
  att_state[0].blendEnable = VK_FALSE;
  att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
  att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
  att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  
  cb.attachmentCount = 1;
  cb.pAttachments = att_state;
  cb.logicOpEnable = VK_FALSE;
  cb.logicOp = VK_LOGIC_OP_NO_OP;
  cb.blendConstants[0] = 1.0f;
  cb.blendConstants[1] = 1.0f;
  cb.blendConstants[2] = 1.0f;
  cb.blendConstants[3] = 1.0f;

  VkPipelineViewportStateCreateInfo vp = {};
  vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vp.pNext = NULL;
  vp.flags = 0;
  vp.viewportCount = NUM_VIEWPORTS;

  dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
  vp.scissorCount = NUM_SCISSORS;
  dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
  vp.pScissors = NULL;
  vp.pViewports = NULL;

  VkPipelineDepthStencilStateCreateInfo ds;
  ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds.pNext = NULL;
  ds.flags = 0;
  ds.depthTestEnable = VK_TRUE;
  ds.depthWriteEnable = VK_TRUE;
  ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  ds.depthBoundsTestEnable = VK_FALSE;
  ds.back.failOp = VK_STENCIL_OP_KEEP;
  ds.back.passOp = VK_STENCIL_OP_KEEP;
  ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
  ds.back.compareMask = 0;
  ds.back.reference = 0;
  ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
  ds.back.writeMask = 0;
  ds.minDepthBounds = 0;
  ds.maxDepthBounds = 0;
  ds.stencilTestEnable = VK_FALSE;
  ds.front = ds.back;

  VkPipelineMultisampleStateCreateInfo ms;
  ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms.pNext = NULL;
  ms.flags = 0;
  ms.pSampleMask = NULL;
  ms.rasterizationSamples = NUM_SAMPLES;
  ms.sampleShadingEnable = VK_FALSE;
  ms.alphaToCoverageEnable = VK_FALSE;
  ms.alphaToOneEnable = VK_FALSE;
  ms.minSampleShading = 0.0;

  VkGraphicsPipelineCreateInfo pipeline;
  pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline.pNext = NULL;
  pipeline.layout = vs->pipeline_layout;
  pipeline.basePipelineHandle = VK_NULL_HANDLE;
  pipeline.basePipelineIndex = 0;
  pipeline.flags = 0;
  pipeline.pVertexInputState = &vi;
  pipeline.pInputAssemblyState = &ia;
  pipeline.pRasterizationState = &rs;
  pipeline.pColorBlendState = &cb;
  pipeline.pTessellationState = NULL;
  pipeline.pMultisampleState = &ms;
  pipeline.pDynamicState = &dynamicState;
  pipeline.pViewportState = &vp;
  pipeline.pDepthStencilState = &ds;
  pipeline.pStages = vs->shaderStages;
  pipeline.stageCount = 2;
  pipeline.renderPass = vs->render_pass;
  pipeline.subpass = 0;

  VkResult res = vkCreateGraphicsPipelines(vs->device, vs->pipeline_cache, 1, &pipeline, NULL, &vs->pipeline);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline\n");
    exit(0);
  }
}

void winvulk_begin_command_buffer(winvulk_vulkan_state* vs) {
    /* DEPENDS on init_command_buffer() */
    VkResult res;

    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = NULL;
    cmd_buf_info.flags = 0;
    cmd_buf_info.pInheritanceInfo = NULL;

    res = vkBeginCommandBuffer(vs->cmd_buffer, &cmd_buf_info);
    if(res != VK_SUCCESS){
      printf("Could not begin command buffer\n");
      exit(0);
    }
}

void winvulk_end_command_buffer(winvulk_vulkan_state* vs) {
    VkResult res;

    res = vkEndCommandBuffer(vs->cmd_buffer);
    if(res != VK_SUCCESS){
      printf("Could not end command buffer\n");
      exit(0);
    }
}

void winvulk_destroy_instance(winvulk_vulkan_state* vs){
  vkDestroyInstance(vs->instance, NULL);
}

void winvulk_destroy_device(winvulk_vulkan_state* vs){
  vkDestroyDevice(vs->device, NULL);
}

void winvulk_destroy_command_buffers(winvulk_vulkan_state* vs){
  vkFreeCommandBuffers(vs->device, vs->cmd_pool, 1, &vs->cmd_buffer);
  vkDestroyCommandPool(vs->device, vs->cmd_pool, NULL);
}

void winvulk_destroy_swapchain(winvulk_vulkan_state* vs){
  for(int i = 0; i < vs->swapchain_image_count;i++){
    vkDestroyImageView(vs->device, vs->swapchain_image_views[i], NULL);
  }
  
  delete[] vs->swapchain_image_views;
  delete[] vs->swapchain_images;
  vkDestroySwapchainKHR(vs->device, vs->swapchain, NULL);
  
  vkDestroySurfaceKHR(vs->instance, vs->surface, NULL);
}

void winvulk_destroy_depth_buffer(winvulk_vulkan_state* vs){
  vkDestroyImageView(vs->device, vs->depth_buffer_view, NULL);
  vkDestroyImage(vs->device, vs->depth_buffer_image, NULL);
  vkFreeMemory(vs->device, vs->depth_buffer_memory, NULL);
}

void winvulk_destroy_uniform_buffer(winvulk_vulkan_state* vs){
  vkDestroyBuffer(vs->device, vs->uniform_buffer, NULL);
  vkFreeMemory(vs->device, vs->uniform_memory, NULL);
}

void winvulk_destroy_descriptor_set_layouts(winvulk_vulkan_state* vs){
  for(int i = 0; i < NUM_DESCRIPTOR_SETS; i++) vkDestroyDescriptorSetLayout(vs->device, vs->desc_layout[i], NULL);
  vkDestroyPipelineLayout(vs->device, vs->pipeline_layout, NULL);
}

void winvulk_destroy_descriptor_set(winvulk_vulkan_state* vs){
  vkDestroyDescriptorPool(vs->device, vs->descriptor_pool, NULL); 
}

void winvulk_destroy_render_pass(winvulk_vulkan_state* vs){
  vkDestroyRenderPass(vs->device, vs->render_pass, NULL);
}

void winvulk_destroy_shaders(winvulk_vulkan_state* vs){
    vkDestroyShaderModule(vs->device, vs->shaderStages[0].module, NULL);
    vkDestroyShaderModule(vs->device, vs->shaderStages[1].module, NULL);
}

void winvulk_destroy_framebuffers(winvulk_vulkan_state* vs){
  for(int i = 0; i < vs->swapchain_image_count; i++){
    vkDestroyFramebuffer(vs->device, vs->framebuffers[i], NULL);
  }

  delete[] vs->framebuffers;
}

void winvulk_destroy_vertex_buffer(winvulk_vulkan_state* vs){
  vkDestroyBuffer(vs->device, vs->vertex_buffer, NULL);
  vkFreeMemory(vs->device, vs->vertex_buffer_memory, NULL);
}

void winvulk_destroy_pipeline(winvulk_vulkan_state* vs){
  vkDestroyPipeline(vs->device, vs->pipeline, NULL);
}

void winvulk_destroy_pipeline_cache(winvulk_vulkan_state* vs){
  vkDestroyPipelineCache(vs->device, vs->pipeline_cache, NULL);
}

void winvulk_render(winvulk_vulkan_state* vs){

  const VkDeviceSize offsets[1] = {0};

  VkClearValue clear_values[2];
  clear_values[0].color.float32[0] = 0.2f;
  clear_values[0].color.float32[1] = 0.2f;
  clear_values[0].color.float32[2] = 0.2f;
  clear_values[0].color.float32[3] = 0.2f;
  clear_values[1].depthStencil.depth = 1.0;
  clear_values[1].depthStencil.stencil = 0;
  
  VkSemaphore imageAcquiredSemaphore;
  VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  imageAcquiredSemaphoreCreateInfo.pNext = NULL;
  imageAcquiredSemaphoreCreateInfo.flags = 0;

  VkResult res = vkCreateSemaphore(vs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
  if(res != VK_SUCCESS){
    printf("Could not create semaphore\n");
    exit(0);
  }

  res = vkAcquireNextImageKHR(vs->device, vs->swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
			      &vs->current_buffer);

  if(res != VK_SUCCESS){
    printf("Could not get next image from swapchain\n");
    exit(0);
  }

  VkRenderPassBeginInfo rp_begin = {};
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = NULL;
  rp_begin.renderPass = vs->render_pass;
  rp_begin.framebuffer = vs->framebuffers[vs->current_buffer];
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = vs->width;
  rp_begin.renderArea.extent.height = vs->height;
  rp_begin.clearValueCount = 2;
  rp_begin.pClearValues = clear_values;

  vkCmdBeginRenderPass(vs->cmd_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(vs->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pipeline);
  vkCmdBindDescriptorSets(vs->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vs->pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
			  vs->descriptor_set, 0, NULL);
  vkCmdBindVertexBuffers(vs->cmd_buffer, 0,
			 1,
			 &vs->vertex_buffer,
			 offsets);

  vs->viewport.height = (float)vs->height;
  vs->viewport.width = (float)vs->width;
  vs->viewport.minDepth = (float)0.0f;
  vs->viewport.maxDepth = (float)1.0f;
  vs->viewport.x = 0;
  vs->viewport.y = 0;
  vkCmdSetViewport(vs->cmd_buffer, 0, NUM_VIEWPORTS, &vs->viewport);

  vs->scissor.extent.width = vs->width;
  vs->scissor.extent.height = vs->height;
  vs->scissor.offset.x = 0;
  vs->scissor.offset.y = 0;
  vkCmdSetScissor(vs->cmd_buffer, 0, NUM_SCISSORS, &vs->scissor);

  vkCmdDraw(vs->cmd_buffer, 12*3, 1, 0, 0);
  vkCmdEndRenderPass(vs->cmd_buffer);
  res = vkEndCommandBuffer(vs->cmd_buffer);

  /* Queue the command buffer for execution */
  const VkCommandBuffer cmd_bufs[] = {vs->cmd_buffer};
  VkFenceCreateInfo fenceInfo;
  VkFence drawFence;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = 0;
  vkCreateFence(vs->device, &fenceInfo, NULL, &drawFence);

  VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info[1] = {};
  submit_info[0].pNext = NULL;
  submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info[0].waitSemaphoreCount = 1;
  submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
  submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
  submit_info[0].commandBufferCount = 1;
  submit_info[0].pCommandBuffers = cmd_bufs;
  submit_info[0].signalSemaphoreCount = 0;
  submit_info[0].pSignalSemaphores = NULL;
    
  res = vkQueueSubmit(vs->graphics_queue, 1, submit_info, drawFence);
  if(res != VK_SUCCESS){
    printf("Could not submit to graphics queue\n");
    exit(0);
  }

  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = NULL;
  present.swapchainCount = 1;
  present.pSwapchains = &vs->swapchain;
  present.pImageIndices = &vs->current_buffer;
  present.pWaitSemaphores = NULL;
  present.waitSemaphoreCount = 0;
  present.pResults = NULL;

  do {
    res = vkWaitForFences(vs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    printf("In waiting loop\n");
  } while (res == VK_TIMEOUT);

  if(res != VK_SUCCESS){
    printf("Drawing was not a success\n");
    exit(0);
  }

  res = vkQueuePresentKHR(vs->present_queue, &present);

  if(res != VK_SUCCESS){
    printf("Could not queue present\n");
    exit(0);
  }
  
  printf("Drawing was a success\n");

  vkDestroyFence(vs->device, drawFence, NULL);

  vkDestroySemaphore(vs->device, imageAcquiredSemaphore, NULL);
}

void winvulk_init_vulkan(winvulk_vulkan_state* vs, Winval* win){
  winvulk_init_instance(vs, win);
  winvulk_find_device(vs);
  winvulk_init_surface(vs, win);
  winvulk_init_device(vs);
  winvulk_init_device_queue(vs);
  winvulk_init_swapchain(vs);
  winvulk_init_command_buffers(vs);
  winvulk_begin_command_buffer(vs);
  winvulk_init_depth_buffer(vs);
  winvulk_init_uniform_buffer(vs);
  winvulk_init_descriptor_set_layouts(vs);
  winvulk_init_render_pass(vs);
  winvulk_init_shaders(vs);
  winvulk_init_framebuffers(vs);
  winvulk_init_vertex_buffer(vs);
  winvulk_init_descriptor_pool(vs);
  winvulk_init_descriptor_set(vs);
  winvulk_init_pipeline_cache(vs);
  winvulk_init_pipeline(vs);


  //winvulk_end_command_buffer(vs);
  winvulk_render(vs);

}

void winvulk_destroy_vulkan(winvulk_vulkan_state* vs){
  winvulk_destroy_pipeline(vs);
  winvulk_destroy_pipeline_cache(vs);
  winvulk_destroy_vertex_buffer(vs);
  winvulk_destroy_framebuffers(vs);
  winvulk_destroy_shaders(vs);
  winvulk_destroy_render_pass(vs);
  winvulk_destroy_descriptor_set(vs);
  winvulk_destroy_descriptor_set_layouts(vs);
  winvulk_destroy_uniform_buffer(vs);
  winvulk_destroy_depth_buffer(vs);
  winvulk_destroy_swapchain(vs);
  winvulk_destroy_command_buffers(vs);
  winvulk_destroy_device(vs);
  winvulk_destroy_instance(vs);
}


#endif //WINVAL_VULKAN_IMPLEMENTATION
