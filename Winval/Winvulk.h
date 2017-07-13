#ifndef INCLUDED_WINVULK
#define INCLUDED_WINVULK

#include <unistd.h>

#define VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan.h"

#include <stdio.h> // printf
#include <vector> //vector
#include <cstdlib> // exit
#include <algorithm> //min, max

#define WINVULK_DEFAULT_WIDTH 1920
#define WINVULK_DEFAULT_HEIGHT 1080


struct winvulk_vulkan_state {
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties device_props;
  VkPhysicalDeviceMemoryProperties device_memory_props;
  VkDevice device;
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
  VkImage*buffers;

  uint width;
  uint height;

  
  std::vector<VkLayerProperties> instance_layer_properties;
  std::vector<const char*> instance_extension_names;
  std::vector<const char*> device_extension_names;
};

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
    printf("Window in use");
  }else if(res == VK_ERROR_INITIALIZATION_FAILED){
    printf("Init failed\n");
  }else if(res == VK_ERROR_EXTENSION_NOT_PRESENT){
    printf("No extension\n");
  }else if(res == VK_ERROR_FEATURE_NOT_PRESENT){
    printf("Feature not here\n");
  }
}
#define GET_INSTANCE_PROC_ADDR(instance, entry)			\
  (PFN_vk##entry)vkGetInstanceProcAddr(instance, "vk" #entry);

#define GET_DEVICE_PROC_ADDR(device, entry)		\
  (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry);

VkResult init_global_extension_properties();
VkResult init_global_layer_properties();


void init_vulkan(winvulk_vulkan_state*, Winval*);

VkResult winvulk_init_instance(winvulk_vulkan_state*, Winval*);
VkResult winvulk_find_device(winvulk_vulkan_state*);
VkResult winvulk_init_device(winvulk_vulkan_state*);
VkResult winvulk_init_command_buffers(winvulk_vulkan_state*);
VkResult winvulk_init_swapchain(winvulk_vulkan_state*, Winval*);

void winvulk_destroy_instance(winvulk_vulkan_state*);
void winvulk_destroy_device(winvulk_vulkan_state*);
void winvulk_destroy_command_buffers(winvulk_vulkan_state*);
void winvukl_destroy_swapchain(winvulk_vulkan_state*);

void winvulk_destroy_vulkan(winvulk_vulkan_state*);


#endif //INCLUDED_WINVULK

#ifdef WINVAL_VULKAN_IMPLEMENTATION


/*VkResult init_global_extension_properties(VkLayerProperties &layer_props) {
    VkExtensionProperties *instance_extensions;
    uint32_t instance_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do {
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
        if (res) return res;

        if (instance_extension_count == 0) {
            return VK_SUCCESS;
        }

        layer_props.extensions.resize(instance_extension_count);
        instance_extensions = layer_props.extensions.data();
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

VkResult init_global_layer_properties(winvulk_vulkan_state* vs) {
    uint32_t instance_layer_count;
    VkLayerProperties *vk_props = NULL;
    VkResult res;

    do {
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        if (res) return res;

        if (instance_layer_count == 0) {
            return VK_SUCCESS;
        }

        vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
    } while (res == VK_INCOMPLETE);

    for (uint32_t i = 0; i < instance_layer_count; i++) {
        layer_properties layer_props;
        layer_props.properties = vk_props[i];
        res = init_global_extension_properties(layer_props);
        if (res) return res;
        vs->instance_layer_properties.push_back(layer_props);
    }
    free(vk_props);

    return res;
    }*/

VkResult winvulk_init_instance(winvulk_vulkan_state* vs, Winval* win){
  vs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  printf("Extension name: %s\n",VK_KHR_SURFACE_EXTENSION_NAME);
  vs->instance_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
  
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
  inst_info.enabledLayerCount = 0;
  inst_info.ppEnabledLayerNames = NULL;

  VkResult res;

  res = vkCreateInstance(&inst_info, NULL, &(vs->instance));

  if( res == VK_ERROR_INCOMPATIBLE_DRIVER){
    printf("Cannot find a compatible Vulkan ICD\n");
    exit(-1);
  }

  /*uint numExts;
  vkEnumerateInstanceExtensionProperties(NULL, &numExts, NULL);
  
  std::vector<VkExtensionProperties> exts(numExts);
  vkEnumerateInstanceExtensionProperties(NULL, &numExts, &exts[0]);
  for(int i =0 ; i < numExts; i++){
    printf("Name: %s\n", exts[i].extensionName);
    }*/
  
  vs->width = WINVULK_DEFAULT_WIDTH;
  vs->height = WINVULK_DEFAULT_HEIGHT;
}

VkResult winvulk_find_device(winvulk_vulkan_state* vs){
  unsigned int device_count;
  
  VkResult res = vkEnumeratePhysicalDevices(vs->instance, &device_count, NULL);
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
  vs->device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  
  VkDeviceQueueCreateInfo queue_info = {};
  float queue_priorities[1] = {1.0f};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;

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

VkResult winvulk_init_swapchain(winvulk_vulkan_state* vs, Winval* win){
  GET_INSTANCE_PROC_ADDR(vs->instance, CreateXlibSurfaceKHR);

  VkXlibSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.flags = 0;
  surface_info.window = win->getWindow();
  surface_info.dpy = win->getDisplay();

  VkResult res = vkCreateXlibSurfaceKHR(vs->instance, &surface_info, NULL, &vs->surface);
  if(res != VK_SUCCESS){
    printf("Could not create surface\n");
    exit(0);
  }

  VkBool32 *pSupportsPresent = (VkBool32*)malloc(vs->queue_family_count * sizeof(VkBool32));
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

  free(pSupportsPresent);

  uint formatCount;
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(vs->physical_device, vs->surface, &formatCount, NULL);
  VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(vs->physical_device, vs->surface, &formatCount, NULL);

  if(res != VK_SUCCESS){
    printf("Could not get formats\n");
    exit(0);
  }

  if(formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
    vs->format = VK_FORMAT_B8G8R8A8_UNORM;
  } else if (formatCount >= 1){
    vs->format = formats[0].format;
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
    printf("Set to current: width = %d, height = %d\n", caps.currentExtent.width, caps.currentExtent.height);
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
    exit(0);
  }

  res = vkGetSwapchainImagesKHR(vs->device, vs->swapchain, &vs->swapchain_image_count, NULL);

  VkImage* swapchainImages = new VkImage[vs->swapchain_image_count];

  res = vkGetSwapchainImagesKHR(vs->device, vs->swapchain, &vs->swapchain_image_count, swapchainImages);

  if(res != VK_SUCCESS){
    printf("Failed to get swapchain images\n");
    exit(0);
  }

  vs->buffers = new VkImage[vs->swapchain_image_count];
  for(int i = 0; i < vs->swapchain_image_count; i++){
    vs->buffers[i] = swapchainImages[i];
  }
  delete[] swapchainImages;
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
  
}


void winvulk_init_vulkan(winvulk_vulkan_state* vs, Winval* win){
  winvulk_init_instance(vs, win);
  winvulk_find_device(vs);
  winvulk_init_device(vs);
  winvulk_init_swapchain(vs, win);
  winvulk_init_command_buffers(vs);
}

void winvulk_destroy_vulkan(winvulk_vulkan_state* vs){
  winvulk_destroy_swapchain(vs);
  winvulk_destroy_command_buffers(vs);
  winvulk_destroy_device(vs);
  winvulk_destroy_instance(vs);
}


#endif //WINVAL_VULKAN_IMPLEMENTATION
