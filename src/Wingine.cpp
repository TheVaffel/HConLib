#include <Wingine.h>
#include <external/glsl_util.h>

#include <cstdlib> // exit

#ifdef WIN32
//Windows...
#undef min
#undef max
//Really, Windows??
#undef near
#undef far
#endif //WIN32

#include <algorithm> //std::min, std::max

#include <FlatAlg.h>

#define wgAssert(B, STR) {if(!(B)){printf("\"%s\" failed at line %d in file %s\n", STR, __LINE__, __FILE__); exit(0);}}


WingineCamera::WingineCamera(float horizontalFOVRadians, float invAspect, float near, float far){
  view = Matrix4(FLATALG_MATRIX_IDENTITY);
  projection = flatalg::projection(horizontalFOVRadians, invAspect, near, far);
  altered = true;
}

void WingineCamera::setPosition(const Vector3& v){
  view[0][3] = -Vector3(view[0][0], view[0][1], view[0][2])*v;
  view[1][3] = -Vector3(view[1][0], view[1][1], view[1][2])*v;
  view[2][3] = -Vector3(view[2][0], view[2][1], view[2][2])*v;
  altered = true;
}

void WingineCamera::setLookAt(const Vector3& pos,
			      const Vector3& target,
			      const Vector3& up){
  view = flatalg::lookAt(pos, target, up);
  altered = true;
}

Matrix4 WingineCamera::getRenderMatrix(){
  if(altered){
    total = clip*projection*view;
    altered = false;
  }

  return ~total;
}

WinginePipelineSetup::WinginePipelineSetup(int numColorAttachments){
  att_state = new VkPipelineColorBlendAttachmentState[numColorAttachments];
}

WinginePipelineSetup::~WinginePipelineSetup(){
  delete[] att_state;
}

WingineRenderPassSetup::WingineRenderPassSetup(int nAttachments){
  numAttachments = nAttachments;
  attachments = new VkAttachmentDescription[numAttachments];
  references = new VkAttachmentReference[numAttachments];
}

WingineRenderPassSetup::~WingineRenderPassSetup(){
  delete [] attachments;
  delete [] references;
}


Wingine::Wingine(const Winval& win){
  initVulkan(&win);
}

Wingine::~Wingine(){
  destroy_vulkan();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunction(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
    size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
  
    printf("[%s] %s\n", pLayerPrefix, pMessage);
    
    return VK_FALSE;
}

VkCommandBuffer Wingine::createCommandBuffer(VkCommandBufferLevel level){
  VkCommandBuffer cmd;
  
  VkCommandBufferAllocateInfo info;
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = NULL;
  info.commandPool = cmd_pool;
  info.level = level;
  info.commandBufferCount = 1;

  VkResult res = vkAllocateCommandBuffers(device, &info, &cmd);
  wgAssert(res == VK_SUCCESS, "Allocate command buffer");
  return cmd;
}

void Wingine::printError(VkResult res){
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

  exit(0);
}


#define GET_INSTANCE_PROC_ADDR(instance, entry)			\
  (PFN_vk##entry)vkGetInstanceProcAddr(instance, "vk" #entry);

#define GET_DEVICE_PROC_ADDR(device, entry)		\
  (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry);

uint32_t Wingine::get_memory_type_index( uint32_t type_bits, VkFlags requirements_mask){
  for(uint32_t i = 0; i < device_memory_props.memoryTypeCount; i++){
    if((type_bits & 1) == 1){
      if((device_memory_props.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask){
	return i;
      }
    }
    type_bits >>= 1;
  }

  //If we got here, appropriate memory type not found
  printf("Appropriate memory type could not be found\n");
  exit(0);
}

VkResult Wingine::init_instance(const  Winval* win){
  VkResult res;

  instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  //instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#ifdef WIN32
  instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else //WIN32
  instance_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif //WIN32

#ifdef DEBUG
  instance_extension_names.push_back("VK_EXT_debug_report");
  instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
  
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = win->getTitle();
  app_info.applicationVersion = 1;
  app_info.pEngineName = "Wingine";
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledExtensionCount = instance_extension_names.size();
  inst_info.ppEnabledExtensionNames = instance_extension_names.data();
  inst_info.enabledLayerCount = instance_layer_names.size();
  inst_info.ppEnabledLayerNames = instance_layer_names.size()? instance_layer_names.data(): NULL;


  res = vkCreateInstance(&inst_info, NULL, &instance);

  if( res == VK_ERROR_INCOMPATIBLE_DRIVER){
    printf("Cannot find a compatible Vulkan ICD\n");
    exit(-1);
  }else if(res != VK_SUCCESS){
    printf("Could not create instance\n");
    exit(-1);
  }

  #ifdef DEBUG
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
    vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if(!vkCreateDebugReportCallbackEXT){
      printf("Did not find callback\n");
    }

    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.flags =  VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = &debugCallbackFunction;
    callbackCreateInfo.pUserData = NULL;

    res = vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, NULL, &debugCallback);
    if(res != VK_SUCCESS){
      printf("Could not create debug callback\n");
      exit(0);
    }
    
  #endif

  width = win->getWidth();
  height = win->getHeight();

  return res;
}

VkResult Wingine::find_device(){
  unsigned int device_count;

  VkResult res;
  vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  VkPhysicalDevice* pDevs = new VkPhysicalDevice[device_count];
  res = vkEnumeratePhysicalDevices(instance, &device_count, pDevs);
  printf("Number of devices: %d\n", device_count);


  bool foundDevice = 0;

  for(uint32_t i = 0; i < device_count; i++){
    vkGetPhysicalDeviceProperties(pDevs[i], &device_props);
    vkGetPhysicalDeviceMemoryProperties(pDevs[i], &device_memory_props);
    printf("Device %i: %s\n", i, device_props.deviceName);

    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &queue_family_count, NULL);
    queue_props = new VkQueueFamilyProperties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &queue_family_count, queue_props);

    for(uint32_t j = 0; j < queue_family_count; j++){
      VkBool32 supportsPresent;
      vkGetPhysicalDeviceSurfaceSupportKHR(pDevs[i], j, surface, &supportsPresent);
      if(supportsPresent && queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
        physical_device = pDevs[i];
      	foundDevice = 1;
      	break;
      }
    }

    delete[] pDevs;

    if(foundDevice)
      break;
  }

  if(!foundDevice){
    printf("Did not find fit device\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_device(){

  VkBool32 *pSupportsPresent = new VkBool32[queue_family_count];
  for(uint32_t i = 0 ; i < queue_family_count; i++){
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &pSupportsPresent[i]);
  }

  graphics_queue_family_index = UINT32_MAX;
  present_queue_family_index = UINT32_MAX;

  for(uint32_t i = 0; i < queue_family_count; i++){
    if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      if(graphics_queue_family_index == UINT32_MAX) graphics_queue_family_index = i;

      if(pSupportsPresent[i] == VK_TRUE){
	present_queue_family_index = i;
	graphics_queue_family_index = i;
	break;
      }
    }
  }

  if(present_queue_family_index == UINT32_MAX){
    for(uint32_t i = 0; i < queue_family_count; i++){
      if(pSupportsPresent[i] == VK_TRUE){
	present_queue_family_index = i;
	break;
      }
    }
  }

  if(present_queue_family_index == UINT32_MAX || graphics_queue_family_index == UINT32_MAX){
    printf("Failed to find present- and/or graphics queue\n");
    exit(0);
  }

  delete[] pSupportsPresent;


  device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceQueueCreateInfo queue_info = {};
  float queue_priorities[1] = {1.0f};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;
  queue_info.queueFamilyIndex = present_queue_family_index;

  VkPhysicalDeviceFeatures features = {};
  features.shaderClipDistance = VK_TRUE;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = device_extension_names.size();
  device_info.ppEnabledExtensionNames = device_extension_names.data();
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = NULL;
  device_info.pEnabledFeatures = &features;

  VkResult res = vkCreateDevice(physical_device, &device_info, NULL, &device);

  if(res != VK_SUCCESS){
    printf("Device creation failed \n");
  }

  return res;
}

VkResult Wingine::init_command_buffers(){
  VkCommandPoolCreateInfo cmd_pool_info = {};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.pNext = NULL;
  cmd_pool_info.queueFamilyIndex = present_queue_family_index;
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkResult res = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);
  if(res != VK_SUCCESS){
    printf("Failed to create command buffer pool \n");
  }

  VkCommandBufferAllocateInfo cmd_alloc = {};
  cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_alloc.pNext = NULL;
  cmd_alloc.commandPool = cmd_pool;
  cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_alloc.commandBufferCount = 1;
  
  res = vkAllocateCommandBuffers(device, &cmd_alloc, &free_command_buffer);
  wgAssert(res == VK_SUCCESS, "Allocate command buffers");

  vkResetCommandBuffer(free_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

  VkFenceCreateInfo fenceInfo;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkCreateFence(device, &fenceInfo, NULL, &free_command_buffer_fence);

  return res;
}

VkResult Wingine::init_surface(const Winval* win){
#ifdef WIN32
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = win->getInstance();
  surface_info.hwnd = win->getHWND();

  VkResult res = vkCreateWin32SurfaceKHR(instance, &surface_info, NULL, &surface);
#else //WIN32
  VkXlibSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.window = win->getWindow();
  surface_info.dpy = win->getDisplay();

  VkResult res = vkCreateXlibSurfaceKHR(instance, &surface_info, NULL, &surface);
#endif //WIN32

  if(res != VK_SUCCESS){
    printf("Could not create surface\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_device_queue(){
  vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);
  if(graphics_queue_family_index == present_queue_family_index){
    present_queue = graphics_queue;
  }else{
    vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
  }

  return VK_SUCCESS;
}

VkResult Wingine::init_swapchain(){

  uint32_t formatCount;
  VkResult res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, NULL);
  if(res != VK_SUCCESS){
    printf("Could not get format count\n");
    exit(0);
  }

  VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, formats);

  if(res != VK_SUCCESS){
    printf("Could not get formats\n");
    exit(0);
  }

  VkColorSpaceKHR colorSpace;

  if(formatCount == 1 &&  formats[0].format == VK_FORMAT_UNDEFINED){
    format = VK_FORMAT_B8G8R8_UNORM;
    colorSpace = formats[0].colorSpace;
  } else{
    format = formats[0].format;
    colorSpace = formats[0].colorSpace;
  }

  delete[] formats;

  VkSurfaceCapabilitiesKHR caps;

  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
  if(res != VK_SUCCESS){
    printf("Could not get surface capabilities\n");
    exit(0);
  }


  VkExtent2D swapchainExtent;

  if(caps.currentExtent.width == 0xFFFFFFFF){ //Set extent to window width and height
    swapchainExtent.width = width;
    swapchainExtent.height = height;

    swapchainExtent.width = std::max(width, caps.minImageExtent.width);
    swapchainExtent.height = std::max(height, caps.minImageExtent.height);

    swapchainExtent.width = std::min(width, caps.maxImageExtent.width);
    swapchainExtent.height = std::min(height, caps.maxImageExtent.height);
  }else{
    swapchainExtent = caps.currentExtent;
    width = swapchainExtent.width;
    height = swapchainExtent.height; //Reevaluate target extent
  }

  uint32_t numSwaps = 2;
  if(numSwaps < caps.minImageCount){
    numSwaps = caps.minImageCount;
  }else if(caps.maxImageCount != 0 && numSwaps > caps.maxImageCount){
    numSwaps = caps.maxImageCount;
  }

  VkSurfaceTransformFlagBitsKHR preTransform;
  if(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR){
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }else{
    preTransform = caps.currentTransform;
  }

  uint32_t presentModeCount;
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, NULL);
  if(res != VK_SUCCESS){
    printf("Could not get present mode count\n");
  }

  VkPresentModeKHR*presentModes = new VkPresentModeKHR[presentModeCount];
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, presentModes);
  if(res != VK_SUCCESS){
    printf("Could not get present modes\n");
    exit(0);
  }


  VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for(uint32_t i = 0; i < presentModeCount; i++){
    if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
      swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    }
  }

  delete [] presentModes;

  VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
  };

  for(uint32_t i = 0; i < sizeof(compositeAlphaFlags)/sizeof(VkCompositeAlphaFlagBitsKHR); i++){
    if(caps.supportedCompositeAlpha & compositeAlphaFlags[i]){
      compositeAlpha = compositeAlphaFlags[i];
      break;
    }
  }

  uint32_t queueFamilyIndices[2] = {graphics_queue_family_index, present_queue_family_index};

  VkSwapchainCreateInfoKHR swapchain_ci = {};
  swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_ci.pNext = NULL;
  swapchain_ci.surface = surface;
  swapchain_ci.minImageCount = numSwaps;
  swapchain_ci.imageFormat = format;
  swapchain_ci.imageExtent = swapchainExtent;
  swapchain_ci.preTransform = preTransform;
  swapchain_ci.compositeAlpha = compositeAlpha;
  swapchain_ci.imageArrayLayers = 1;
  swapchain_ci.presentMode = swapchainPresentMode;
  swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
  swapchain_ci.clipped = true;
  swapchain_ci.imageColorSpace = colorSpace;
  swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_ci.queueFamilyIndexCount = 0;
  swapchain_ci.pQueueFamilyIndices = NULL;

  if(graphics_queue_family_index != present_queue_family_index){
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
    printf("Got different family queues for graphics and presenting, setting concurrent mode in swapchain\n");
  }

  res = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swapchain);

  if(res != VK_SUCCESS){
    printf("Failed to create swapchain\n");
    printError(res);
    exit(0);
  }

  res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);

  swapchain_images = new VkImage[swapchain_image_count];

  res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);

  if(res != VK_SUCCESS){
    printf("Failed to get swapchain images\n");
    exit(0);
  }

  swapchain_image_views = new VkImageView[swapchain_image_count];
  for(uint32_t i = 0; i < swapchain_image_count; i++){
    VkImageViewCreateInfo color_image_view = {};
    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = NULL;
    color_image_view.flags = 0;
    color_image_view.image = swapchain_images[i];
    color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_image_view.format = format;
    color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
    color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
    color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
    color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
    color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_image_view.subresourceRange.baseMipLevel = 0;
    color_image_view.subresourceRange.levelCount = 1;
    color_image_view.subresourceRange.baseArrayLayer = 0;
    color_image_view.subresourceRange.layerCount = 1;

    res = vkCreateImageView(device, &color_image_view, NULL, &swapchain_image_views[i]);
    if(res != VK_SUCCESS){
      printf("Could not create image view for image %d\n", i);
    }
  }

  VkFenceCreateInfo fenceInfo;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkCreateFence(device, &fenceInfo, NULL, &imageAcquiredFence);

  VkSemaphoreCreateInfo semaphoreInfo;
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreInfo.pNext = NULL;
  semaphoreInfo.flags = 0;
  
  for(int i = 0; i < 10; i++){ // start with some small amount, expand if necessary
    VkSemaphore sem;
    res = vkCreateSemaphore(device, &semaphoreInfo, NULL, &sem);
    drawSemaphores.push_back(sem);
  }
  currSemaphore = 0;
  
  if(res != VK_SUCCESS){
    printf("Could not create image fence\n");
    exit(0);
  }
 
  return res;
}

VkResult Wingine::init_depth_buffer(){
  VkImageCreateInfo image_info = {};

  const VkFormat depth_format = VK_FORMAT_D16_UNORM;
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(physical_device, depth_format, &props);
  
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
  image_info.extent.width = width;
  image_info.extent.height = height;
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

  depth_buffer_format = depth_format;

  VkResult res = vkCreateImage(device, &image_info, NULL, &depth_buffer_image);
  if(res != VK_SUCCESS){
    printf("Could not create depth buffer\n");
    exit(0);
  }

  VkMemoryRequirements mem_reqs = {};
  vkGetImageMemoryRequirements(device, depth_buffer_image, &mem_reqs);
  mem_alloc.allocationSize = mem_reqs.size;

  mem_alloc.memoryTypeIndex = get_memory_type_index(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  res = vkAllocateMemory(device, &mem_alloc, NULL, &depth_buffer_memory);
  if(res != VK_SUCCESS){
    printf("Memory could not be allocated\n");
    exit(0);
  }

  res = vkBindImageMemory(device, depth_buffer_image, depth_buffer_memory, 0);
  if(res != VK_SUCCESS){
    printf("Memory could not be bound\n");
    exit(0);
  }

  view_info.image = depth_buffer_image;
  res = vkCreateImageView(device, &view_info, NULL, &depth_buffer_view);
  if(res != VK_SUCCESS){
    printf("Depth image view could not be created\n");
    exit(0);
  }

  return res;

}

VkResult Wingine::init_descriptor_pool(){
  VkDescriptorPoolSize type_count[2];
  type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count[0].descriptorCount = UNIFORM_DESCRIPTOR_POOL_SIZE;
  
  type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  type_count[1].descriptorCount = TEXTURE_DESCRIPTOR_POOL_SIZE;

  VkDescriptorPoolCreateInfo descriptor_pool_info = {};
  descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool_info.pNext = NULL;
  descriptor_pool_info.flags = 0;
  descriptor_pool_info.maxSets = TEXTURE_DESCRIPTOR_POOL_SIZE + UNIFORM_DESCRIPTOR_POOL_SIZE;
  descriptor_pool_info.poolSizeCount = 2;
  descriptor_pool_info.pPoolSizes = type_count;

  VkResult res = vkCreateDescriptorPool(device, &descriptor_pool_info, NULL, &descriptor_pool);
  if(res != VK_SUCCESS){
    printf("Could not create descriptor pool\n");
    exit(0);
  }

  return res;
}

void Wingine::render_pass_setup_generic(WingineRenderPassSetup* setup){
  setup->attachments[0].format = format;
  setup->attachments[0].samples = NUM_SAMPLES;
  setup->attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  setup->attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  setup->attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  setup->attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  setup->attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  setup->attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  setup->attachments[0].flags = 0;

  setup->attachments[1].format = depth_buffer_format;
  setup->attachments[1].samples = NUM_SAMPLES;
  setup->attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  setup->attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  setup->attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  setup->attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  setup->attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  setup->attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  setup->attachments[1].flags = 0;

  setup->references[0].attachment = 0;
  setup->references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  if(setup->numAttachments > 1){
    setup->references[1].attachment = 1;
    setup->references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }
  setup->subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  setup->subpass.flags = 0;
  setup->subpass.inputAttachmentCount = 0;
  setup->subpass.pInputAttachments = NULL;
  setup->subpass.colorAttachmentCount = 1;
  setup->subpass.pColorAttachments = &setup->references[0];
  setup->subpass.pResolveAttachments = NULL;
  setup->subpass.pDepthStencilAttachment = &setup->references[1];
  setup->subpass.preserveAttachmentCount = 0;
  setup->subpass.pPreserveAttachments = NULL;

  setup->createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  setup->createInfo.pNext = NULL;
  setup->createInfo.flags = 0;
  setup->createInfo.attachmentCount = 2;
  setup->createInfo.pAttachments = setup->attachments;
  setup->createInfo.subpassCount = 1;
  setup->createInfo.pSubpasses = &setup->subpass;
  setup->createInfo.dependencyCount = 0;
  setup->createInfo.pDependencies = NULL;

}

VkResult Wingine::init_render_passes(){
  WingineRenderPassSetup setup;
  render_pass_setup_generic(&setup);
  
  VkResult res = vkCreateRenderPass(device, &setup.createInfo, NULL, &render_pass_generic);
  if(res != VK_SUCCESS){
    printf("Creating generic render pass\n");
    exit(0);
  }

  setup.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  setup.attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

  if(res != VK_SUCCESS){
    printf("Could not create render pass\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_framebuffers(){
  VkImageView attachments[2];
  attachments[1] = depth_buffer_view;

  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = render_pass_generic;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = attachments;
  fb_info.width = width;
  fb_info.height = height;
  fb_info.layers = 1;
  framebuffers = new VkFramebuffer[swapchain_image_count];

  VkResult res;
  for(uint32_t i = 0 ; i < swapchain_image_count; i++){
    attachments[0] = swapchain_image_views[i];
    res = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);
    if(res != VK_SUCCESS){
      printf("Could not create framebuffer number %d\n", i);
      exit(0);
    }
  }

  return res;
}

VkResult Wingine::init_pipeline_cache(){
  VkPipelineCacheCreateInfo pipelineCache;
  pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCache.pNext = NULL;
  pipelineCache.initialDataSize = 0;
  pipelineCache.pInitialData = NULL;
  pipelineCache.flags = 0;
  VkResult res = vkCreatePipelineCache(device, &pipelineCache, NULL, &pipeline_cache);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline cache\n");
  }

  return res;
}

void Wingine::destroy_instance(){
#ifdef DEBUG
  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;
  vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
  
  vkDestroyDebugReportCallbackEXT(instance, debugCallback, NULL);
#endif //DEBUG
  
  vkDestroyInstance(instance, NULL);
}

void Wingine::destroy_device(){
  vkDestroyDevice(device, NULL);
}

void Wingine::destroy_command_buffers(){
  /*for(int i = 0; i < MAX_NUM_COMMANDS; i++){
    vkFreeCommandBuffers(device, cmd_pool, 1, &(cmd_buffers[i]));
    }*/
  vkDestroyCommandPool(device, cmd_pool, NULL);
  vkDestroyFence(device, free_command_buffer_fence, NULL);
}

void Wingine::destroy_swapchain(){
  for(uint32_t i = 0; i < swapchain_image_count;i++){
    vkDestroyImageView(device, swapchain_image_views[i], NULL);
  }

  delete[] swapchain_image_views;
  delete[] swapchain_images;

  vkWaitForFences(device, 1, &imageAcquiredFence, VK_TRUE, 1000000000ULL);
  vkDestroyFence(device, imageAcquiredFence, NULL);

  for(uint32_t i = 0; i < drawSemaphores.size(); i++){
    vkDestroySemaphore(device, drawSemaphores[i], NULL);
  }
  
  vkDestroySwapchainKHR(device, swapchain, NULL);

  vkDestroySurfaceKHR(instance, surface, NULL);
}

void Wingine::destroy_depth_buffer(){
  vkDestroyImageView(device, depth_buffer_view, NULL);
  vkDestroyImage(device, depth_buffer_image, NULL);
  vkFreeMemory(device, depth_buffer_memory, NULL);
}

void Wingine::destroy_render_passes(){
  vkDestroyRenderPass(device, render_pass_generic, NULL);
}

void Wingine::destroy_framebuffers(){
  for(uint32_t i = 0; i < swapchain_image_count; i++){
    vkDestroyFramebuffer(device, framebuffers[i], NULL);
  }

  delete[] framebuffers;
}

void Wingine::destroy_pipeline(){
  vkDestroyPipeline(device, color_pipeline, NULL);
}

void Wingine::destroy_pipeline_cache(){
  vkDestroyPipelineCache(device, pipeline_cache, NULL);
}

void Wingine::destroy_descriptor_pool(){
  vkDestroyDescriptorPool(device, descriptor_pool, NULL);
}


WingineBuffer Wingine::createBuffer( uint32_t usage, uint32_t size){
  WingineBuffer wBuffer;

  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = usage;
  buf_info.size = size;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  VkResult res = vkCreateBuffer(device, &buf_info, NULL, &wBuffer.buffer);
  if(res != VK_SUCCESS){
    printf("Could not create buffer\n");
    exit(0);
  }

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(device, wBuffer.buffer, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  alloc_info.memoryTypeIndex = get_memory_type_index(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); //Ensures reachability from host

  if(alloc_info.memoryTypeIndex < 0){
    printf("Could not find proper memory\n");
    exit(0);
  }

  res = vkAllocateMemory(device, &alloc_info, NULL, &wBuffer.memory);
  if(res != VK_SUCCESS){
    printf("Could not allocate memory\n");
    exit(0);
  }

  res = vkBindBufferMemory(device, wBuffer.buffer, wBuffer.memory, 0);
  if(res != VK_SUCCESS){
    printf("Could not bind buffer and memory\n");
  }

  return wBuffer;
}

void Wingine::destroyBuffer(const WingineBuffer& buffer){
  vkDestroyBuffer(device, buffer.buffer, NULL);
  vkFreeMemory(device, buffer.memory, NULL);
}

VkResult Wingine::setBuffer( const WingineBuffer& buffer, const void* data, uint32_t size){
  void * map;
  VkResult res;

  res = vkMapMemory(device, buffer.memory, 0, VK_WHOLE_SIZE, 0, &map);
  if(res != VK_SUCCESS){
    printf("Could not map memory\n");
    exit(0);
  }

  memcpy(map, data, size);

  vkUnmapMemory(device, buffer.memory);

  return res;
}

WingineUniform Wingine::createUniform(uint32_t size){
  WingineUniform uniform;
  uniform.buffer = createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size);
  uniform.bufferInfo.buffer = uniform.buffer.buffer;
  uniform.bufferInfo.offset = 0;
  uniform.bufferInfo.range = size;

  return uniform;
}

void Wingine::setUniform(const WingineUniform& uniform, void* data, uint32_t size){
  setBuffer(uniform.buffer, data, size);
}

void Wingine::destroyUniform(const WingineUniform& uniform){
  destroyBuffer(uniform.buffer);
}

//Serves as basis for other pipelines
void Wingine::pipeline_setup_generic(WinginePipelineSetup* setup, int numVertexAttribs){
  memset(setup->dynamicStateEnables, 0, sizeof(setup->dynamicStateEnables));
  setup->dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  setup->dynamicState.pNext = NULL;
  setup->dynamicState.flags = 0;
  setup->dynamicState.pDynamicStates = setup->dynamicStateEnables;
  setup->dynamicState.dynamicStateCount = 0;
  
  setup->vi_bindings = new VkVertexInputBindingDescription[numVertexAttribs];
  setup->vi_attribs = new VkVertexInputAttributeDescription[numVertexAttribs];

  setup->vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  setup->vi.pNext = NULL;
  setup->vi.flags = 0;
  setup->vi.vertexBindingDescriptionCount = numVertexAttribs;
  setup->vi.pVertexBindingDescriptions = setup->vi_bindings;
  
  setup->vi.vertexAttributeDescriptionCount = numVertexAttribs;
  setup->vi.pVertexAttributeDescriptions = setup->vi_attribs;

  setup->ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  setup->ia.pNext = NULL;
  setup->ia.flags = 0;
  setup->ia.primitiveRestartEnable = VK_FALSE;
  setup->ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  setup->rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  setup->rs.pNext = NULL;
  setup->rs.flags = 0;
  setup->rs.polygonMode = VK_POLYGON_MODE_FILL;
  setup->rs.cullMode = VK_CULL_MODE_BACK_BIT;
  setup->rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
  setup->rs.depthClampEnable = VK_FALSE;
  setup->rs.rasterizerDiscardEnable = VK_FALSE;
  setup->rs.depthBiasEnable = VK_FALSE;
  setup->rs.depthBiasConstantFactor = 0;
  setup->rs.depthBiasClamp = 0;
  setup->rs.depthBiasSlopeFactor = 0;
  setup->rs.lineWidth = 1.0f;

  setup->cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  setup->cb.pNext = NULL;
  setup->cb.flags = 0;

  setup->att_state[0].colorWriteMask = 0xf;
  setup->att_state[0].blendEnable = VK_TRUE;
  setup->att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
  setup->att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
  setup->att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  setup->att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  setup->att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  setup->att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  
  setup->cb.attachmentCount = 1;
  setup->cb.pAttachments = setup->att_state;
  setup->cb.logicOpEnable = VK_FALSE;
  setup->cb.logicOp = VK_LOGIC_OP_NO_OP;
  setup->cb.blendConstants[0] = 1.0f;
  setup->cb.blendConstants[1] = 1.0f;
  setup->cb.blendConstants[2] = 1.0f;
  setup->cb.blendConstants[3] = 1.0f;

  setup->vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  setup->vp.pNext = NULL;
  setup->vp.flags = 0;
  setup->vp.viewportCount = NUM_VIEWPORTS;

  setup->dynamicStateEnables[setup->dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
  setup->vp.scissorCount = NUM_SCISSORS;
  setup->dynamicStateEnables[setup->dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
  setup->vp.pScissors = NULL;
  setup->vp.pViewports = NULL;

  setup->ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  setup->ds.pNext = NULL;
  setup->ds.flags = 0;
  setup->ds.depthTestEnable = VK_TRUE;
  setup->ds.depthWriteEnable = VK_TRUE;
  setup->ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  setup->ds.depthBoundsTestEnable = VK_FALSE;
  setup->ds.back.failOp = VK_STENCIL_OP_KEEP;
  setup->ds.back.passOp = VK_STENCIL_OP_KEEP;
  setup->ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
  setup->ds.back.compareMask = 0;
  setup->ds.back.reference = 0;
  setup->ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
  setup->ds.back.writeMask = 0;
  setup->ds.minDepthBounds = 0;
  setup->ds.maxDepthBounds = 0;
  setup->ds.stencilTestEnable = VK_FALSE;
  setup->ds.front = setup->ds.back;

  setup->ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  setup->ms.pNext = NULL;
  setup->ms.flags = 0;
  setup->ms.pSampleMask = NULL;
  setup->ms.rasterizationSamples = NUM_SAMPLES;
  setup->ms.sampleShadingEnable = VK_FALSE;
  setup->ms.alphaToCoverageEnable = VK_FALSE;
  setup->ms.alphaToOneEnable = VK_FALSE;
  setup->ms.minSampleShading = 0.0;

  setup->layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  setup->layoutCreateInfo.pNext = NULL;
  setup->layoutCreateInfo.flags = 0;
  setup->layoutCreateInfo.pushConstantRangeCount = 0;
  setup->layoutCreateInfo.pPushConstantRanges = NULL;
  setup->layoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
  setup->layoutCreateInfo.pSetLayouts = NULL; // Replaced in createPipeline

  setup->createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  setup->createInfo.pNext = NULL;
  setup->createInfo.layout = {}; // Replaced in createPipeline
  setup->createInfo.basePipelineHandle = VK_NULL_HANDLE;
  setup->createInfo.basePipelineIndex = 0;
  setup->createInfo.flags = 0;
  setup->createInfo.pVertexInputState = &setup->vi;
  setup->createInfo.pInputAssemblyState = &setup->ia;
  setup->createInfo.pRasterizationState = &setup->rs;
  setup->createInfo.pColorBlendState = &setup->cb;
  setup->createInfo.pTessellationState = NULL;
  setup->createInfo.pMultisampleState = &setup->ms;
  setup->createInfo.pDynamicState = &setup->dynamicState;
  setup->createInfo.pViewportState = &setup->vp;
  setup->createInfo.pDepthStencilState = &setup->ds;
  setup->createInfo.pStages = NULL; // Replaced in createPipeline
  setup->createInfo.stageCount = 0;
  setup->createInfo.renderPass = render_pass_generic;
  setup->createInfo.subpass = 0;

  render_pass_setup_generic(&setup->renderPassSetup);
}

void Wingine::pipeline_setup_color_renderer( WinginePipelineSetup* setup){
  pipeline_setup_generic(setup, 2);
}

void Wingine::create_pipeline_color_renderer( VkPipeline* pipeline){
  WinginePipelineSetup setup;
  pipeline_setup_color_renderer(&setup);

  VkResult res = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &setup.createInfo, NULL, pipeline);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline\n");
    exit(0);
  }
}

void Wingine::create_pipeline_color_renderer_single_buffer( VkPipeline* pipeline){
  WinginePipelineSetup setup;
  pipeline_setup_color_renderer(&setup);
  
  setup.vi_bindings[0].stride = 2*4*sizeof(float);
  
  setup.vi_attribs[1].binding = 0;
  setup.vi_attribs[1].offset = 4*sizeof(float);

  setup.vi.vertexBindingDescriptionCount = 1;

  VkResult res = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &setup.createInfo, NULL, pipeline);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline\n");
    exit(0);
  }
}

void Wingine::initVulkan(const Winval* win){
  init_instance( win);
  init_surface( win);
  find_device();
  init_device();
  init_device_queue();
  init_command_buffers();
  init_swapchain();
  init_depth_buffer();
  init_render_passes();
  init_framebuffers();
  init_descriptor_pool();
  init_pipeline_cache();
  stage_next_image();

  //create_pipeline_color_renderer( &color_pipeline);
}

void Wingine::destroy_vulkan(){
  destroy_pipeline();
  destroy_pipeline_cache();
  destroy_framebuffers();
  destroy_render_passes();
  destroy_depth_buffer();
  destroy_descriptor_pool();
  destroy_swapchain();
  destroy_command_buffers();
  destroy_device();
  destroy_instance();
}

WingineResourceSetLayout Wingine::createResourceSetLayout(int numResources, VkDescriptorType* types,  VkShaderStageFlagBits* stages){
  WingineResourceSetLayout wgLayout;
  
  VkDescriptorSetLayoutBinding* lbs = new VkDescriptorSetLayoutBinding[numResources];

  for(int i = 0; i < numResources; i++){
    lbs[i] = {};
    lbs[i].binding = i;
    lbs[i].descriptorCount = 1;
    lbs[i].stageFlags = stages[i];
    lbs[i].descriptorType = types[i];
    lbs[i].pImmutableSamplers = NULL;
  }
  
  VkDescriptorSetLayoutCreateInfo dlc = {};
  dlc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  dlc.pNext = NULL;
  dlc.flags = 0;
  dlc.bindingCount = numResources;
  dlc.pBindings = lbs;
  
  VkResult res = vkCreateDescriptorSetLayout(device, &dlc, NULL, &wgLayout.layout);
  wgAssert(res == VK_SUCCESS, "Creating descriptor set layout");

  delete[] lbs;

  wgLayout.numResources = numResources;

  return wgLayout;
}

void Wingine::destroyResourceSetLayout(WingineResourceSetLayout wrsl){
  vkDestroyDescriptorSetLayout(device,wrsl.layout,NULL);
}

WingineResourceSet Wingine::createResourceSet(WingineResourceSetLayout resourceLayout, WingineResource** resources){
  WingineResourceSet resourceSet;

  //Create descriptor set and initialize descriptors
  VkDescriptorSetAllocateInfo alloc;
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.pNext = NULL;
  alloc.descriptorPool = descriptor_pool;
  alloc.descriptorSetCount  = 1;
  alloc.pSetLayouts = &resourceLayout.layout;

  VkResult res = vkAllocateDescriptorSets(device, &alloc, &resourceSet.descriptorSet);
  wgAssert(res == VK_SUCCESS, "Allocate descriptor set");
  
  VkWriteDescriptorSet* writes = new VkWriteDescriptorSet[resourceLayout.numResources];

  for(int i = 0; i < resourceLayout.numResources; i++){
    writes[i] = {};
    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[i].pNext = NULL;
    writes[i].dstSet = resourceSet.descriptorSet;
    writes[i].descriptorCount = 1;
    writes[i].descriptorType = resources[i]->getDescriptorType();
    writes[i].pBufferInfo = resources[i]->getBufferInfo();
    writes[i].pImageInfo = resources[i]->getImageInfo();
    writes[i].dstArrayElement = 0;
    writes[i].dstBinding = i;
  }
  vkUpdateDescriptorSets(device, resourceLayout.numResources, writes, 0, NULL);

  return resourceSet;
}

void Wingine::destroyResourceSet(const WingineResourceSet& set){
}

WingineShader Wingine::createShader(const char* shaderText, VkShaderStageFlagBits stageBit){
  WingineShader wgShader;
  wgShader.shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  wgShader.shader.pNext = NULL;
  wgShader.shader.pSpecializationInfo = NULL;
  wgShader.shader.flags = 0;
  wgShader.shader.stage = stageBit;
  wgShader.shader.pName = "main";

  glslang::InitializeProcess();
  std::vector<unsigned int> spirvVector;
  
  bool retVal = GLSLtoSPV(stageBit, shaderText, spirvVector);
 
  wgAssert(retVal, "Compiling");
  
  VkShaderModuleCreateInfo mc;
  mc.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  mc.pNext = NULL;
  mc.flags = 0;
  mc.codeSize = spirvVector.size() * sizeof(unsigned int);
  mc.pCode = spirvVector.data();

  VkResult res = vkCreateShaderModule(device, &mc, NULL, &wgShader.shader.module);
  
  wgAssert(res == VK_SUCCESS, "Creating shader module");

  glslang::FinalizeProcess();

  return wgShader;
}

void Wingine::destroyShader(WingineShader shader){
  vkDestroyShaderModule(device, shader.shader.module, NULL);
  //delete[] shader.uniformSets;
}

WinginePipeline Wingine::createPipeline(WingineResourceSetLayout resourceLayout, int numShaders, WingineShader* shaders, int numVertexAttribs, VkFormat* attribTypes, bool clear){

  WinginePipelineSetup pipelineSetup;
  WinginePipeline pipeline;
  wgAssert(numVertexAttribs <= MAX_VERTEX_ATTRIBUTES, "Max Vertex Attributes high enough");
  pipeline.numVertexAttribs = numVertexAttribs;
  pipeline_setup_generic(&pipelineSetup, numVertexAttribs);

  if(clear){
    pipelineSetup.renderPassSetup.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    if(pipelineSetup.renderPassSetup.numAttachments > 1){
      pipelineSetup.renderPassSetup.attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
  }

  //Todo: Let vertex attribs be of variable size? (Currently 4*sizeof(float) only)
  VkPipelineShaderStageCreateInfo* mods = new VkPipelineShaderStageCreateInfo[numShaders];
  for(int i=  0; i < numShaders; i++){
    mods[i] = shaders[i].shader;
  }

  for(int i = 0; i < numVertexAttribs; i++){

    pipelineSetup.vi_bindings[i].binding = i;
    pipelineSetup.vi_bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    pipelineSetup.vi_bindings[i].stride = 4*sizeof(float);

    pipelineSetup.vi_attribs[i].binding = i;
    pipelineSetup.vi_attribs[i].location = i;
    pipelineSetup.vi_attribs[i].format = attribTypes[i]; //This is a bit too static
    pipelineSetup.vi_attribs[i].offset = 0;
  }
  
  pipelineSetup.createInfo.pStages = mods;
  pipelineSetup.createInfo.stageCount = numShaders;
  
  pipeline.descriptorSetLayout = resourceLayout.layout;
  
  pipelineSetup.layoutCreateInfo.setLayoutCount = 1;
  pipelineSetup.layoutCreateInfo.pSetLayouts = &resourceLayout.layout;
  
  VkResult res = vkCreatePipelineLayout(device, &pipelineSetup.layoutCreateInfo, NULL, &pipeline.pipelineLayout);
  wgAssert(res == VK_SUCCESS, "Create pipeline layout");
  
  res = vkCreateRenderPass(device, &pipelineSetup.renderPassSetup.createInfo, NULL, &pipeline.compatibleRenderPass);
  wgAssert(res == VK_SUCCESS, "Creating pipelineLayout");

  pipelineSetup.createInfo.layout =  pipeline.pipelineLayout;
  
  res = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipelineSetup.createInfo, NULL, &pipeline.pipeline);
  wgAssert(res == VK_SUCCESS, "Creating pipeline");

  delete[] mods;
  
  return pipeline;
}

void Wingine::destroyPipeline(WinginePipeline pipeline){
  vkDestroyPipelineLayout(device, pipeline.pipelineLayout, NULL);
  vkDestroyRenderPass(device, pipeline.compatibleRenderPass, NULL);
  vkDestroyPipeline(device, pipeline.pipeline, NULL);
}

void Wingine::wg_cmd_set_image_layout(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStages, VkPipelineStageFlags destStages){
  //Shameful copy-paste from LunarG. I'm sorry :(
  
  VkImageMemoryBarrier image_memory_barrier = {};
  image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_memory_barrier.pNext = NULL;
  image_memory_barrier.srcAccessMask = 0;
  image_memory_barrier.dstAccessMask = 0;
  image_memory_barrier.oldLayout = oldLayout;
  image_memory_barrier.newLayout = newLayout;
  image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_memory_barrier.image = image;
  image_memory_barrier.subresourceRange.aspectMask = aspect;
  image_memory_barrier.subresourceRange.baseMipLevel = 0;
  image_memory_barrier.subresourceRange.levelCount = 1;
  image_memory_barrier.subresourceRange.baseArrayLayer = 0;
  image_memory_barrier.subresourceRange.layerCount = 1;

  switch (oldLayout) {
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_PREINITIALIZED:
    image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    break;

  default:
    break;
  }

  switch (newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;

  default:
    break;
  }

  vkCmdPipelineBarrier(cmd, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
}

void Wingine::copyImage(int w, int h, VkImage srcImage, VkImageLayout srcStartLayout, VkImageLayout srcEndLayout, VkImage dstImage, VkImageLayout dstStartLayout, VkImageLayout dstEndLayout){
  VkResult res;
  
  VkCommandBufferBeginInfo cmd_begin = {};
  cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin.pNext = NULL;
  cmd_begin.flags = 0;
  cmd_begin.pInheritanceInfo = NULL;
  
  //Ensure free command buffer is ready
  do{
    res = vkWaitForFences(device, 1, &free_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  wgAssert(res == VK_SUCCESS, "Waiting for free_command_buffer_fence");

  res = vkResetFences(device, 1, &free_command_buffer_fence);
  wgAssert(res == VK_SUCCESS, "Reset free_command_buffer_fence")
  
  res = vkBeginCommandBuffer(free_command_buffer, &cmd_begin);
  wgAssert(res == VK_SUCCESS, "Begin command buffer for copying texture image");

  // Set the layouts of the images for transfer
  wg_cmd_set_image_layout(free_command_buffer, srcImage ,VK_IMAGE_ASPECT_COLOR_BIT, srcStartLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  wg_cmd_set_image_layout(free_command_buffer, dstImage, VK_IMAGE_ASPECT_COLOR_BIT, dstStartLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

  // Transfer
  VkImageCopy copy;
  copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copy.srcSubresource.mipLevel = 0;
  copy.srcSubresource.baseArrayLayer = 0;
  copy.srcSubresource.layerCount = 1;
  copy.srcOffset.x = 0;
  copy.srcOffset.y = 0;
  copy.srcOffset.z = 0;
  copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copy.dstSubresource.mipLevel = 0;
  copy.dstSubresource.baseArrayLayer = 0;
  copy.dstSubresource.layerCount = 1;
  copy.dstOffset.x = 0;
  copy.dstOffset.y = 0;
  copy.dstOffset.z = 0;
  copy.extent.width = w;
  copy.extent.height = h;
  copy.extent.depth = 1;

  vkCmdCopyImage(free_command_buffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

  // Set the layouts of the images for use later
  if(srcEndLayout != VK_IMAGE_LAYOUT_UNDEFINED){
    wg_cmd_set_image_layout(free_command_buffer, srcImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcEndLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  }
  
  wg_cmd_set_image_layout(free_command_buffer, dstImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstEndLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  res = vkEndCommandBuffer(free_command_buffer);
  wgAssert(res == VK_SUCCESS, "End free command buffer");

  VkSubmitInfo submitInfo = {};
  
  submitInfo.pNext = NULL;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = NULL;
  submitInfo.pWaitDstStageMask = NULL;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &free_command_buffer;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = NULL;

  res = vkQueueSubmit(graphics_queue,1,&submitInfo, free_command_buffer_fence);
  wgAssert(res == VK_SUCCESS, "Submitting command buffer for copying image");

  //Arh, or else, we must take care of the images, which I don't want to :(
  do{
    res = vkWaitForFences(device, 1, &free_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);
}

WingineImage Wingine::createImage(uint32_t w, uint32_t h, VkImageLayout layout, VkImageUsageFlags usage){
  WingineImage image;
  image.width = w;
  image.height = h;
  image.layout = layout;

  VkImageCreateInfo ici = {};
  ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ici.pNext = NULL;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = VK_FORMAT_R8G8B8A8_UNORM;
  ici.extent.width = w;
  ici.extent.height = h;
  ici.extent.depth = 1;
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = NUM_SAMPLES;
  ici.tiling = VK_IMAGE_TILING_LINEAR;
  ici.initialLayout = layout;
  ici.usage = usage;
  ici.queueFamilyIndexCount = 0;
  ici.pQueueFamilyIndices = NULL;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.flags = 0;

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.pNext = NULL;
  memAlloc.allocationSize = 0;
  memAlloc.memoryTypeIndex = 0;

  VkMemoryRequirements memReqs;

  VkResult res = vkCreateImage(device, &ici, NULL, &image.image);
  wgAssert(res == VK_SUCCESS, "Create image");

  vkGetImageMemoryRequirements(device, image.image, &memReqs);

  memAlloc.allocationSize = memReqs.size;
  
  memAlloc.memoryTypeIndex = get_memory_type_index(memReqs.memoryTypeBits,
						   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  
  res = vkAllocateMemory(device, &memAlloc, NULL, &image.mem);
  wgAssert(res == VK_SUCCESS, "Allocate image memory");

  res = vkBindImageMemory(device, image.image, image.mem, 0);
  wgAssert(res == VK_SUCCESS, "Bind image memory");

  
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.image = image.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  res = vkCreateImageView(device,&viewInfo, NULL, &image.view);

  
  image.imageInfo.imageLayout = image.layout;
  image.imageInfo.imageView = image.view;

  return image;
}

void Wingine::destroyImage(WingineImage w){
  vkDestroyImageView(device, w.view, NULL);
  vkDestroyImage(device, w.image, NULL);
  vkFreeMemory(device, w.mem, NULL);
}

//This will create a texture immutable from the CPU
WingineTexture Wingine::createTexture(int w, int h, unsigned char* imageBuffer)
{
  VkResult res;

  WingineTexture resultTexture;

  resultTexture.image.width = w;
  resultTexture.image.height = h;

  //First, prepare staging image
  VkImageCreateInfo ici = {};
  ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ici.pNext = NULL;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = VK_FORMAT_R8G8B8A8_UNORM;
  ici.extent.width = w;
  ici.extent.height = h;
  ici.extent.depth = 1;
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = NUM_SAMPLES;
  ici.tiling = VK_IMAGE_TILING_LINEAR;
  ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  ici.queueFamilyIndexCount = 0;
  ici.pQueueFamilyIndices = NULL;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.flags = 0;

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.pNext = NULL;
  memAlloc.allocationSize = 0;
  memAlloc.memoryTypeIndex = 0;

  VkImage mappableImage;
  VkDeviceMemory mappableMemory;
  VkMemoryRequirements memReqs;

  res = vkCreateImage(device, &ici, NULL, &mappableImage);
  wgAssert(res == VK_SUCCESS, "Create image");

  vkGetImageMemoryRequirements(device, mappableImage, &memReqs);

  memAlloc.allocationSize = memReqs.size;
  
  memAlloc.memoryTypeIndex = get_memory_type_index(memReqs.memoryTypeBits,
						   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  res = vkAllocateMemory(device, &memAlloc, NULL, &mappableMemory);
  wgAssert(res == VK_SUCCESS, "Allocate image memory");

  res = vkBindImageMemory(device, mappableImage, mappableMemory, 0);
  wgAssert(res == VK_SUCCESS, "Bind image memory");


  VkImageSubresource subres = {};
  subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subres.mipLevel = 0;
  subres.arrayLayer = 0;

  VkSubresourceLayout layout;
  void *data;

  vkGetImageSubresourceLayout(device, mappableImage, &subres, &layout);

  res = vkMapMemory(device, mappableMemory, 0,memReqs.size, 0, &data);
  wgAssert(res == VK_SUCCESS, "Map memory");

  unsigned char* currRow = (unsigned char*)data;
  for(int i = 0; i < h; i++){
    memcpy(currRow, &imageBuffer[4*i*w], w*4);
    currRow += layout.rowPitch;
  }

  vkUnmapMemory(device, mappableMemory);

  //Now, create the actual image
  resultTexture.image = createImage(w,h,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

  
  copyImage(w, h, mappableImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_UNDEFINED, resultTexture.image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


  //Initialize sampler
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1;
  samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  res = vkCreateSampler(device, &samplerInfo, NULL, &resultTexture.sampler);
  wgAssert(res == VK_SUCCESS, "Creating image sampler");

  resultTexture.image.imageInfo.imageLayout = resultTexture.image.layout;
  resultTexture.image.imageInfo.imageView = resultTexture.image.view;
  resultTexture.image.imageInfo.sampler = resultTexture.sampler;

  vkDestroyImage(device, mappableImage, NULL);
  vkFreeMemory(device, mappableMemory, NULL);

  return resultTexture;
}

void Wingine::destroyTexture(WingineTexture& tex){  
  vkDestroySampler(device, tex.sampler, NULL);
  destroyImage(tex.image);
}

void Wingine::stage_next_image(){
  VkResult res;
  res = vkResetFences(device, 1, &imageAcquiredFence);
  
  res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
			      VK_NULL_HANDLE,
			      imageAcquiredFence,
			      &current_buffer);
  res = vkQueueWaitIdle(graphics_queue);
  currSemaphore = 0;
  
  if(res != VK_SUCCESS){
    printf("Could not get next image from swapchain\n");
    exit(0);
  }
}

void Wingine::present(){

  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &current_buffer;
  presentInfo.pWaitSemaphores = drawSemaphores.data();
  presentInfo.waitSemaphoreCount = currSemaphore;
  presentInfo.pResults = NULL;

  VkResult res = vkQueuePresentKHR(present_queue, &presentInfo);

  if(res != VK_SUCCESS){
    printf("Could not queue present\n");
    exit(0);
  }

  stage_next_image();
}

int Wingine::getScreenWidth() const{
  return width;
}

int Wingine::getScreenHeight() const{
  return height;
}

void Wingine::pushNewDrawSemaphore(){
  VkSemaphore sem;
  VkSemaphoreCreateInfo inf;
  inf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  inf.pNext = NULL;
  inf.flags = 0;
  
  VkResult res = vkCreateSemaphore(device, &inf, NULL, &sem);
  wgAssert(res == VK_SUCCESS, "Create semaphore");
  drawSemaphores.push_back(sem);
}

VkFramebuffer Wingine::getCurrentFramebuffer() const {
  return framebuffers[current_buffer];
}

void Wingine::setScene(WingineScene& scene){
  currentScene = &scene;
}

void Wingine::renderScene(){
  for(uint32_t i = 0; i < currentScene->objectGroups.size(); i++){
    //if(currentScene->objectGroups[i].altered){ // We have to rerecord to get the right framebuffer :(
    currentScene->objectGroups[i].startRecordingCommandBuffer();
    for(uint32_t j = 0;j < currentScene->objectGroups[i].objects.size(); j++){
      //if(currentScene->objectGroups[i].objects[j].isAltered()){
	currentScene->objectGroups[i].objects[j].recordCommandBuffer(currentScene->objectGroups[i].commandBuffer);
	//}
    }
    currentScene->objectGroups[i].endRecordingCommandBuffer();
      //}
    submitDrawCommandBuffer(currentScene->objectGroups[i].commandBuffer);
  }

  present();
}

void Wingine::submitDrawCommandBuffer(const VkCommandBuffer& buffer){
  VkResult res;
  
  do{
    res = vkWaitForFences(device, 1, &imageAcquiredFence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  if(currSemaphore == drawSemaphores.size()){
    pushNewDrawSemaphore();
  }

  VkSubmitInfo sinfo = {};
  sinfo.pNext = NULL;
  sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinfo.waitSemaphoreCount = 0;
  sinfo.pWaitSemaphores = NULL;
  sinfo.pWaitDstStageMask = NULL;
  sinfo.commandBufferCount = 1;
  sinfo.pCommandBuffers = &buffer;
  sinfo.signalSemaphoreCount = 1;
  sinfo.pSignalSemaphores = &drawSemaphores[currSemaphore];

  res = vkQueueSubmit(graphics_queue, 1, &sinfo, VK_NULL_HANDLE);
  
  currSemaphore++;
}

WingineRenderObject::WingineRenderObject(int numInds, int numVertexAttribs, WingineBuffer* buffers, const WingineBuffer& iBuffer, const WingineResourceSet& uSet){
  for(int i = 0; i < numVertexAttribs; i++){
    vertexAttribs.push_back(buffers[i]);
  }
  indexBuffer = iBuffer;
  uniformSet = uSet;
  numDrawIndices = numInds;
  indexOffset = 0;
}

void WingineRenderObject::setIndexOffset(int newIndex){
  indexOffset = newIndex;
}

void WingineRenderObject::setPipeline(const WinginePipeline& p){
  pipeline = &p;
}

void WingineRenderObject::setIndexBuffer(const WingineBuffer& ib){
  indexBuffer = ib;
  altered = true;
}

void WingineRenderObject::setVertexAttribs(const WingineBuffer& wb, int index){
  vertexAttribs[index] = wb;
  altered = true;
}

void WingineRenderObject::recordCommandBuffer(VkCommandBuffer& cmd){

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout,
			  0, 1, &uniformSet.descriptorSet, 0, NULL);
  VkBuffer* vertexBuffers = new VkBuffer[vertexAttribs.size()];
  for(uint32_t i = 0; i< vertexAttribs.size(); i++){
    vertexBuffers[i] = vertexAttribs[i].buffer;
  }
  VkDeviceSize offsets[] = {0, 0};
  vkCmdBindVertexBuffers(cmd, 0, vertexAttribs.size(), vertexBuffers, offsets);
  delete[] vertexBuffers;

  vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmd, numDrawIndices, 1, indexOffset, 0, 0);
  //vkEndCommandBuffer(cmd);
  altered = false;
}

void WingineRenderObject::setCommandBuffer(const VkCommandBuffer& cmd){
  commandBuffer = cmd;
}

VkCommandBuffer* WingineRenderObject::getCommandBufferPointer(){
  return &commandBuffer;
}

void WingineRenderObject::setObjectGroup(WingineObjectGroup& wog){
  objectGroup = &wog;
}

bool WingineRenderObject::isAltered(){
  return altered;
}

WingineScene::WingineScene(Wingine& w){
  wg = &w;
}

WingineScene::~WingineScene(){
  for(uint32_t i = 0; i < objectGroups.size(); i++){
    wg->destroyPipeline(objectGroups[i].pipeline);
  }
}

void WingineScene::addPipeline(WingineResourceSetLayout layout, int numShaders, WingineShader* shaders, int numVertexAttribs, VkFormat* attribTypes){
  WinginePipeline p =  wg->createPipeline(layout, numShaders, shaders, numVertexAttribs, attribTypes, objectGroups.size() == 0);
  WingineObjectGroup wog(*wg);
  wog.pipeline = p;
  wog.shouldClearAttachments = objectGroups.size() == 0;
  wog.commandBuffer = wg->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  objectGroups.push_back(wog);
}

void WingineScene::addObject(const WingineRenderObject& obj, int pipelineInd){
  objectGroups[pipelineInd].objects.push_back(obj);
  int index = objectGroups[pipelineInd].objects.size()-1;
  objectGroups[pipelineInd].objects[index].setPipeline(objectGroups[pipelineInd].pipeline);
  objectGroups[pipelineInd].altered = true;
  objectGroups[pipelineInd].objects[index].setCommandBuffer(wg->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY));
  objectGroups[pipelineInd].objects[index].setObjectGroup(objectGroups[pipelineInd]); 
}

WingineObjectGroup::WingineObjectGroup(const Wingine& wg){
  wingine = &wg;
}

void WingineObjectGroup::startRecordingCommandBuffer(){
  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo begin = {};
  begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin.pNext = NULL;
  begin.flags = 0;
  begin.pInheritanceInfo = NULL;
    
  VkClearValue clear_values[2];
  clear_values[0].color.float32[0] = 0.2f;
  clear_values[0].color.float32[1] = 0.2f;
  clear_values[0].color.float32[2] = 0.2f;
  clear_values[0].color.float32[3] = 1.0f;
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0.0f;

  VkViewport viewport;
  viewport.height = (float)wingine->getScreenHeight();
  viewport.width = (float)wingine->getScreenWidth();
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  viewport.x = 0;
  viewport.y = 0;

  VkRect2D scissor;
  scissor.extent.width = wingine->getScreenWidth();
  scissor.extent.height = wingine->getScreenHeight();
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  VkRenderPassBeginInfo rp_begin = {};
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = NULL;
  rp_begin.renderPass = pipeline.compatibleRenderPass;
  rp_begin.clearValueCount = shouldClearAttachments? 2 : 0; //TODO: Allow other than just two writing attachments
  
  rp_begin.pClearValues = clear_values; // Hopefully, this is ignored if render pass does no clearing
  rp_begin.framebuffer = wingine->getCurrentFramebuffer();
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = wingine->getScreenWidth();
  rp_begin.renderArea.extent.height = wingine->getScreenHeight();

  VkResult res = vkBeginCommandBuffer(commandBuffer, &begin);
  wgAssert(res == VK_SUCCESS, "Begin command buffer");
  
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  
  vkCmdBeginRenderPass(commandBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
}


void WingineObjectGroup::endRecordingCommandBuffer(){
  vkCmdEndRenderPass(commandBuffer);
  VkResult res;
  res = vkEndCommandBuffer(commandBuffer);
  wgAssert(res == VK_SUCCESS, "End command buffer");
}
