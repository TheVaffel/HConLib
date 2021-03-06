#include <Wingine.hpp>

#ifdef WINGINE_WITH_GLSLANG
#include <glsl_util.h>
#endif // WINGINE_WITH_GLSLANG

#include <string.h> // mem*
#include <cstdlib> // exit
#include <fstream> // .obj file reading
#include <sstream> // file content processing

#ifdef WIN32
//Windows...
#undef min
#undef max
//Really, Windows??
#undef near
#undef far
#endif //WIN32

#include <algorithm> //std::min, std::max
#include <cmath>

#include <FlatAlg.hpp>

using namespace falg;

#define wgAssert(B, STR) {if(!(B)){printf("\"%s\" failed at line %d in file %s\n", STR, __LINE__, __FILE__); exit(0);}}

WingineCamera::WingineCamera(float horizontalFOVRadians, float invAspect, float near, float far){
  view = Mat4(FLATALG_MATRIX_IDENTITY);
  // projection = flatalg::projection(horizontalFOVRadians, invAspect, near, far);
  projection = Mat4(FLATALG_MATRIX_PROJECTION, horizontalFOVRadians, invAspect, near, far);
  altered = true;
}

void WingineCamera::setPosition(const Vec3& v){
  view(0, 3) = -Vec3(view(0, 0), view(0, 1), view(0, 2))*v;
  view(1, 3) = -Vec3(view(1, 0), view(1, 1), view(1, 2))*v;
  view(2, 3) = -Vec3(view(2, 0), view(2, 1), view(2, 2))*v;
  altered = true;
}

void WingineCamera::setLookAt(const Vec3& pos,
			      const Vec3& target,
			      const Vec3& up){
  // view = flatalg::lookAt(pos, target, up);
  view = Mat4(FLATALG_MATRIX_LOOK_AT, pos, target, up);
  altered = true;
}

void WingineCamera::setLookDirection(float rightAngle, float downAngle,
				     const Vec3& up){
  float cd = cos(-downAngle);
  Vec3 dir(sin(-rightAngle) * cd, sin(-downAngle), cos(-rightAngle) * cd);
  Vec3 right = cross(dir, up).normalized();
  Vec3 viewUp = cross(right, dir);

  Vec3 pos = (~view.submatrix<3, 3>(0, 0))*-Vec3(view(0, 3), view(1, 3), view(2, 3));
  
  view = Mat4(right.x(), right.y(), right.z(), 0.f,
		 viewUp.x(), viewUp.y(), viewUp.z(), 0.f,
		 -dir.x(), -dir.y(), -dir.z(), 0.f,
		 0.f, 0.f, 0.f, 1.f);
  setPosition(pos);
}

Mat4 WingineCamera::getRenderMatrix(){
  return ~getTransformMatrix();
}

Mat4 WingineCamera::getTransformMatrix(){
  if(altered){
    total = clip*projection*view;
    altered = false;
  }

  return total;
}

Mat4 WingineCamera::getViewMatrix(){
  return view;
}

Vec3 WingineCamera::getForwardVector() {
  return Vec3(-view(2, 0), -view(2, 1), -view(2, 2));
}

Vec3 WingineCamera::getRightVector() {
  return Vec3(view(0, 0), view(0, 1), view(0, 2));
}

Vec3 WingineCamera::getUpVector() {
  return Vec3(view(1, 0), view(1, 1), view(1, 2));
}

Vec3 WingineCamera::getPosition() {
  return (~view.submatrix<3, 3>(0, 0))*-Vec3(view(0, 3), view(1, 3), view(2, 3));
}

WinginePipelineSetup::WinginePipelineSetup(std::initializer_list<WgAttachmentType> types)
  : WinginePipelineSetup(types.size(), std::begin(types)) {};

WinginePipelineSetup::WinginePipelineSetup(int numAttachments, const WgAttachmentType* types)
  : renderPassSetup(numAttachments, types){
  int numColorAtt = 0;
  for(int i = 0; i < numAttachments; i++){
    if(types[i] == WG_ATTACHMENT_TYPE_COLOR)
      numColorAtt++;
  }
  if(numColorAtt)
    att_state = new VkPipelineColorBlendAttachmentState[numColorAtt];
  else {
    att_state = NULL;
    cb.attachmentCount = 0;
  }
}

WinginePipelineSetup::~WinginePipelineSetup(){
  if(att_state)
    delete[] att_state;
}

WingineRenderPassSetup::WingineRenderPassSetup(std::initializer_list<WgAttachmentType> types)
  : WingineRenderPassSetup(types.size(), std::begin(types)){
}

WingineRenderPassSetup::WingineRenderPassSetup(int nAttachments, const WgAttachmentType* types){
  numAttachments = nAttachments;
  attachmentTypes = new WgAttachmentType[nAttachments];
  for(int i = 0; i < nAttachments; i++){
    attachmentTypes[i] = types[i];
  }
  attachments = new VkAttachmentDescription[numAttachments];
  references = new VkAttachmentReference[numAttachments];
}

WingineRenderPassSetup::~WingineRenderPassSetup(){
  delete [] attachmentTypes;
  delete [] attachments;
  delete [] references;
}

#if WIN32
Wingine::Wingine(int inWidth, int inHeight,
                 const char* title,
                 HINSTANCE hinst, HWND hwnd){
  initVulkan(inWidth, inHeight, title,
             hinst, hwnd);
}

#else

Wingine::Wingine(int inWidth, int inHeight,
		 const char* title,
		 Window window, Display* display) {
  initVulkan(inWidth, inHeight, title,
	     window, display);
}

#endif // WIN32

Wingine::Wingine(const Winval& win){
  initVulkan(win.getWidth(), win.getHeight(), win.getTitle(),
#ifdef WIN32
             win.getInstance(), win.getHWND());
#else // WIN32
             win.getWindow(), win.getDisplay());
#endif // WIN32
}


#ifdef WIN32
void Wingine::initVulkan(int inWidth, int inHeight, const char* title,
			 HINSTANCE hinstance, HWND hwnd)
#else // WIN32
  void Wingine::initVulkan(int inWidth, int inHeight, const char* title,
			   Window window, Display* display)
#endif // WIN32
{

  init_instance(inWidth, inHeight, title);

#ifdef WIN32
  init_surface( hinstance, hwnd);
#else // WIN32
  init_surface(window, display);
#endif // WIN32

  find_device();
  init_device();
  init_device_queue();
  init_command_buffers();
  init_swapchain();
  init_render_passes();
  init_framebuffers();
  init_descriptor_pool();
  init_pipeline_cache();
  stage_next_image();
}

Wingine::~Wingine(){
  destroy_vulkan();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunction(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
    size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData ) {

  printf("[%s] Code: %d Message: %s\n", pLayerPrefix, messageCode, pMessage);

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

VkResult Wingine::init_instance(int inWidth, int inHeight, const char * title){
  VkResult res;

  instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
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
  app_info.pApplicationName = title;
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

  width = inWidth;
  height = inHeight;

  return res;
}

VkResult Wingine::find_device(){
  unsigned int device_count;

  VkResult res;
  vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  VkPhysicalDevice* pDevs = new VkPhysicalDevice[device_count];
  res = vkEnumeratePhysicalDevices(instance, &device_count, pDevs);
  wgAssert(res == VK_SUCCESS, "Finding devices");
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
  compute_queue_family_index = UINT32_MAX;

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

  for(uint32_t i = 0; i < queue_family_count; i++){
    if(queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
      compute_queue_family_index = i;
    }
  }

  wgAssert(compute_queue_family_index != UINT32_MAX, "Find compute capable queue\n");

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

  cmd_pool_info.queueFamilyIndex = compute_queue_family_index;

  res = vkCreateCommandPool(device, &cmd_pool_info, NULL, &compute_cmd_pool);

  wgAssert(res == VK_SUCCESS, "Create compute command pool");

  VkCommandBufferAllocateInfo cmd_alloc = {};
  cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_alloc.pNext = NULL;
  cmd_alloc.commandPool = cmd_pool;
  cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_alloc.commandBufferCount = 1;

  res = vkAllocateCommandBuffers(device, &cmd_alloc, &free_command_buffer);
  wgAssert(res == VK_SUCCESS, "Allocate command buffer");

  cmd_alloc.commandPool = compute_cmd_pool;
  res = vkAllocateCommandBuffers(device, &cmd_alloc, &compute_command_buffer);
  wgAssert(res == VK_SUCCESS, "Allocate compute command buffer");

  vkResetCommandBuffer(free_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  vkResetCommandBuffer(compute_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

  VkFenceCreateInfo fenceInfo;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  vkCreateFence(device, &fenceInfo, NULL, &free_command_buffer_fence);
  vkCreateFence(device, &fenceInfo, NULL, &compute_command_buffer_fence);
  return res;
}

#ifdef WIN32
VkResult Wingine::init_surface(HINSTANCE hinst, HWND hwnd){
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = hinst;
  surface_info.hwnd = hwnd;

  VkResult res = vkCreateWin32SurfaceKHR(instance, &surface_info, NULL, &surface);
  if(res != VK_SUCCESS){
    printf("Could not create surface\n");
    exit(0);
  }

  return res;
}

#else //WIN32

VkResult Wingine::init_surface(Window window, Display* display){
  VkXlibSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.window = window;
  surface_info.dpy = display;

  VkResult res = vkCreateXlibSurfaceKHR(instance, &surface_info, NULL, &surface);

  if(res != VK_SUCCESS){
    printf("Could not create surface\n");
    exit(0);
  }

  return res;
}

#endif //WIN32

VkResult Wingine::init_device_queue(){
  vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);
  if(graphics_queue_family_index == present_queue_family_index){
    present_queue = graphics_queue;
  }else{
    vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
  }

  vkGetDeviceQueue(device,compute_queue_family_index,0,&compute_queue);

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
  swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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

VkResult Wingine::init_descriptor_pool(){
  const int typeCount = 3;
  VkDescriptorPoolSize type_size[typeCount];
  type_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_size[0].descriptorCount = UNIFORM_DESCRIPTOR_POOL_SIZE;

  type_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  type_size[1].descriptorCount = TEXTURE_DESCRIPTOR_POOL_SIZE;

  type_size[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  type_size[2].descriptorCount = IMAGE_STORE_DESCRIPTOR_POOL_SIZE;

  VkDescriptorPoolCreateInfo descriptor_pool_info = {};
  descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool_info.pNext = NULL;
  descriptor_pool_info.flags = 0;
  descriptor_pool_info.maxSets = TEXTURE_DESCRIPTOR_POOL_SIZE + UNIFORM_DESCRIPTOR_POOL_SIZE;
  descriptor_pool_info.poolSizeCount = typeCount;
  descriptor_pool_info.pPoolSizes = type_size;

  VkResult res = vkCreateDescriptorPool(device, &descriptor_pool_info, NULL, &descriptor_pool);
  if(res != VK_SUCCESS){
    printf("Could not create descriptor pool\n");
    exit(0);
  }

  return res;
}

void Wingine::render_pass_setup_generic(WingineRenderPassSetup* setup){
  VkImageLayout finalLayoutMap[] =
    {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  VkImageLayout layoutMap[] =
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  int numOfType[NUM_WG_ATTACHMENT_TYPES];

  for(int i = 0; i < NUM_WG_ATTACHMENT_TYPES; i++){
    numOfType[i] = 0;
  }

  for(int i = 0; i < setup->numAttachments; i++){
    setup->attachments[i].samples = NUM_SAMPLES;
    // Is this right? Might get problems when we want the depth buffer to persist between passes
    setup->attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    setup->attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    setup->attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    setup->attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    setup->attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    setup->attachments[i].finalLayout = finalLayoutMap[setup->attachmentTypes[i]];

    setup->attachments[i].flags = 0;

    switch(setup->attachmentTypes[i]){
      case WG_ATTACHMENT_TYPE_COLOR:
        setup->attachments[i].format = format;
        break;
      case WG_ATTACHMENT_TYPE_DEPTH:
        setup->attachments[i].format = DEPTH_BUFFER_FORMAT;
        break;
      default:
        printf("Unrecognized attachment type in render_pass_setup_generic\n");
        printf("If you are adding a new attachment, make sure the rest of this"
          " function makes sense\n");
        exit(-1);
    }

    numOfType[setup->attachmentTypes[i]]++;
  }

  int currTop = 0;
  int startForType[NUM_WG_ATTACHMENT_TYPES];
  int currOfType[NUM_WG_ATTACHMENT_TYPES];
  for(int i = 0; i < NUM_WG_ATTACHMENT_TYPES; i++){
    startForType[i] = currTop;
    currTop += numOfType[i];
    currOfType[i] = 0; // Used in next loop
  }

  for(int i = 0; i < setup->numAttachments; i++){
    int index = startForType[setup->attachmentTypes[i]] + currOfType[setup->attachmentTypes[i]];
    setup->references[index].attachment = i;
    setup->references[index].layout = layoutMap[setup->attachmentTypes[i]];
    currOfType[setup->attachmentTypes[i]]++;
  }

  setup->subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  setup->subpass.flags = 0;
  setup->subpass.inputAttachmentCount = 0;
  setup->subpass.pInputAttachments = NULL;
  setup->subpass.colorAttachmentCount = numOfType[WG_ATTACHMENT_TYPE_COLOR];
  setup->subpass.pColorAttachments = &setup->references[startForType[WG_ATTACHMENT_TYPE_COLOR]];
  setup->subpass.pResolveAttachments = NULL;
  setup->subpass.pDepthStencilAttachment = &setup->references[startForType[WG_ATTACHMENT_TYPE_DEPTH]];
  setup->subpass.preserveAttachmentCount = 0;
  setup->subpass.pPreserveAttachments = NULL;

  setup->createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  setup->createInfo.pNext = NULL;
  setup->createInfo.flags = 0;
  setup->createInfo.attachmentCount = setup->numAttachments;
  setup->createInfo.pAttachments = setup->attachments;
  setup->createInfo.subpassCount = 1;
  setup->createInfo.pSubpasses = &setup->subpass;
  setup->createInfo.dependencyCount = 0;
  setup->createInfo.pDependencies = NULL;

}

VkResult Wingine::init_render_passes(){
  WingineRenderPassSetup setup({WG_ATTACHMENT_TYPE_COLOR,
			 WG_ATTACHMENT_TYPE_DEPTH});
  render_pass_setup_generic(&setup);

  VkResult res = vkCreateRenderPass(device, &setup.createInfo, NULL, &render_pass_generic);
  if(res != VK_SUCCESS){
    printf("Could not create generic render pass\n");
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
  framebuffers = new WingineFramebuffer[swapchain_image_count];

  for(uint32_t i = 0; i < swapchain_image_count; i++){
    framebuffers[i] = create_framebuffer_from_vk_image(swapchain_images[i], width, height, true);
  }

  return VK_SUCCESS;
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
  vkDestroyCommandPool(device, cmd_pool, NULL);
  vkDestroyCommandPool(device, compute_cmd_pool, NULL);
  vkDestroyFence(device, free_command_buffer_fence, NULL);
  vkDestroyFence(device, compute_command_buffer_fence, NULL);
}

void Wingine::destroy_swapchain(){

  delete[] swapchain_images;

  vkWaitForFences(device, 1, &imageAcquiredFence, VK_TRUE, 1000000000ULL);
  vkDestroyFence(device, imageAcquiredFence, NULL);

  for(uint32_t i = 0; i < drawSemaphores.size(); i++){
    vkDestroySemaphore(device, drawSemaphores[i], NULL);
  }

  vkDestroySwapchainKHR(device, swapchain, NULL);

  vkDestroySurfaceKHR(instance, surface, NULL);
}

void Wingine::destroy_render_passes(){
  vkDestroyRenderPass(device, render_pass_generic, NULL);
}

void Wingine::destroy_framebuffers(){
  for(uint32_t i = 0; i < swapchain_image_count; i++){
    vkDestroyImageView(device, framebuffers[i].image.view, NULL);
    destroyImage(framebuffers[i].depth_image);

    vkDestroyFramebuffer(device, framebuffers[i].framebuffer, NULL);
  }

  delete[] framebuffers;
}

void Wingine::destroyFramebuffer(WingineFramebuffer buffer){
  if(buffer.image.image)
    destroyImage(buffer.image);
  destroyImage(buffer.depth_image);
  vkDestroyFramebuffer(device, buffer.framebuffer, NULL);
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

void Wingine::destroy_pipeline_cache(){
  vkDestroyPipelineCache(device, pipeline_cache, NULL);
}

void Wingine::destroy_descriptor_pool(){
  vkDestroyDescriptorPool(device, descriptor_pool, NULL);
}

WingineBuffer Wingine::createVertexBuffer(uint32_t size, const void* data){
  return createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, data);
}

WingineBuffer Wingine::createIndexBuffer(uint32_t size, const void* data){
  return createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, data);
}

WingineBuffer Wingine::createBuffer( uint32_t usage, uint32_t size, const void* data){
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

  if(data){
    setBuffer(wBuffer, data, size);
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

void Wingine::image_create_info_generic(VkImageCreateInfo* ici){
  ici->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ici->pNext = NULL;
  ici->imageType = VK_IMAGE_TYPE_2D;
  ici->format = VK_FORMAT_B8G8R8A8_UNORM;
  ici->extent.width = width;
  ici->extent.height = height;
  ici->extent.depth = 1;
  ici->mipLevels = 1;
  ici->arrayLayers = 1;
  ici->samples = NUM_SAMPLES;
  ici->tiling = VK_IMAGE_TILING_OPTIMAL;
  ici->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  ici->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  ici->queueFamilyIndexCount = 0;
  ici->pQueueFamilyIndices = NULL;
  ici->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici->flags = 0;
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

  if(setup->att_state){
    setup->att_state[0].colorWriteMask = 0xf;
    setup->att_state[0].blendEnable = VK_TRUE;
    setup->att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
    setup->att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    setup->att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    setup->att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    setup->att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    setup->att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

    setup->cb.attachmentCount = 1;
  }else{
    setup->cb.attachmentCount = 0;
  }
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
  WinginePipelineSetup setup({WG_ATTACHMENT_TYPE_COLOR,
	WG_ATTACHMENT_TYPE_DEPTH});
  pipeline_setup_color_renderer(&setup);

  VkResult res = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &setup.createInfo, NULL, pipeline);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline\n");
    exit(0);
  }
}

void Wingine::create_pipeline_color_renderer_single_buffer( VkPipeline* pipeline){
  WinginePipelineSetup setup({WG_ATTACHMENT_TYPE_COLOR,
	WG_ATTACHMENT_TYPE_DEPTH});
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

void Wingine::destroy_vulkan(){
  destroy_pipeline_cache();
  destroy_framebuffers();
  destroy_render_passes();
  destroy_descriptor_pool();
  destroy_swapchain();
  destroy_command_buffers();
  destroy_device();
  destroy_instance();
}

VkDescriptorType Wingine::get_descriptor_type(WgResourceType type){
  switch(type){
    case WG_RESOURCE_TYPE_TEXTURE:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case WG_RESOURCE_TYPE_UNIFORM:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case WG_RESOURCE_TYPE_STORE_IMAGE:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
      printf("Resource type not recognized\n");
      exit(-1);
  }
}

WingineResourceSetLayout Wingine::createResourceSetLayout(std::initializer_list<WgResourceType> types,
							  std::initializer_list<WgShaderStage> stages){
   wgAssert(types.size() == stages.size(), "Arguments to createResourceSetLayout has different size");
   return createResourceSetLayout(types.size(), std::begin(types), (VkShaderStageFlagBits*)std::begin(stages));
}
  

WingineResourceSetLayout Wingine::createResourceSetLayout(std::initializer_list<WgResourceType> types,
							  std::initializer_list<VkShaderStageFlagBits> stages){
  wgAssert(types.size() == stages.size(), "Arguments to createResourceSetLayout has different size");
  return createResourceSetLayout(types.size(), std::begin(types), std::begin(stages));
}

WingineResourceSetLayout Wingine::createResourceSetLayout(int numResources, const WgResourceType* types,  const VkShaderStageFlagBits* stages){
  WingineResourceSetLayout wgLayout;

  VkDescriptorSetLayoutBinding* lbs = new VkDescriptorSetLayoutBinding[numResources];
  wgLayout.types = new WgResourceType[numResources];
  for(int i = 0; i < numResources; i++){
    wgLayout.types[i] = types[i];
  }

  for(int i = 0; i < numResources; i++){
    lbs[i] = {};
    lbs[i].binding = i;
    lbs[i].descriptorCount = 1;
    lbs[i].stageFlags = stages[i];
    lbs[i].descriptorType = get_descriptor_type(types[i]);
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
  delete[] wrsl.types;
  vkDestroyDescriptorSetLayout(device,wrsl.layout,NULL);
}

WingineResourceSet Wingine::createResourceSet(WingineResourceSetLayout& resourceLayout){
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
  resourceSet.layout = &resourceLayout;
  
  return resourceSet;
}

void Wingine::updateResourceSet(WingineResourceSet& set, std::initializer_list<WingineResource const *> resources){
  wgAssert(resources.size() == set.layout->numResources, "Resource set and number of resources coincide");
  updateResourceSet(set, std::begin(resources));
}

void Wingine::updateResourceSet(WingineResourceSet& set, const WingineResource* const * resources){
  VkWriteDescriptorSet* writes = new VkWriteDescriptorSet[set.layout->numResources];

  for(uint32_t i = 0; i < set.layout->numResources; i++){
    writes[i] = {};
    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[i].pNext = NULL;
    writes[i].dstSet = set.descriptorSet;
    writes[i].descriptorCount = 1;
    writes[i].descriptorType = get_descriptor_type(set.layout->types[i]);
    writes[i].pBufferInfo = resources[i]->getBufferInfo();
    writes[i].pImageInfo = resources[i]->getImageInfo();
    writes[i].dstArrayElement = 0;
    writes[i].dstBinding = i;
  }
  
  vkUpdateDescriptorSets(device, set.layout->numResources, writes, 0, NULL);

  delete [] writes;
}

void Wingine::updateResourceSetIndex(WingineResourceSet& set, WingineResource* resource, int index){
  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = NULL;
  write.dstSet = set.descriptorSet;
  write.descriptorCount = 1;
  write.descriptorType = get_descriptor_type(set.layout->types[index]);
  write.pBufferInfo = resource->getBufferInfo();
  write.pImageInfo = resource->getImageInfo();
  write.dstArrayElement = 0;
  write.dstBinding = index;

  vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void Wingine::destroyResourceSet(const WingineResourceSet& set){
}

#ifdef WINGINE_WITH_GLSLANG
WingineShader Wingine::createVertexShader(const char* shaderText){
  return createShader(shaderText, WG_SHADER_STAGE_VERTEX);
}

WingineShader Wingine::createFragmentShader(const char* shaderText){
  return createShader(shaderText, WG_SHADER_STAGE_FRAGMENT);
}

WingineShader Wingine::createShader(const char* shaderText, WgShaderStage stageBit){

  glslang::InitializeProcess();
  std::vector<uint32_t> spirvVector;
  bool retVal = GLSLtoSPV((VkShaderStageFlagBits) stageBit, shaderText, spirvVector);
  
  glslang::FinalizeProcess();
  
  wgAssert(retVal, "Compiling");

  return createShader(spirvVector, stageBit);
}
#endif // WINGINE_WITH_GLSLANG

// Create shader module directly from SPIR-V binary
WingineShader Wingine::createShader(const std::vector<uint32_t>& spirv, WgShaderStage stageBit){
  WingineShader wgShader;
  wgShader.shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  wgShader.shader.pNext = NULL;
  wgShader.shader.pSpecializationInfo = NULL;
  wgShader.shader.flags = 0;
  wgShader.shader.stage = (VkShaderStageFlagBits) stageBit;
  wgShader.shader.pName = "main";

  VkShaderModuleCreateInfo mc;
  mc.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  mc.pNext = NULL;
  mc.flags = 0;
  mc.codeSize = spirv.size() * sizeof(uint32_t);
  mc.pCode = spirv.data();

  VkResult res = vkCreateShaderModule(device, &mc, NULL, &wgShader.shader.module);

  wgAssert(res == VK_SUCCESS, "Creating shader module");

  return wgShader;
}

void Wingine::destroyShader(WingineShader shader){
  vkDestroyShaderModule(device, shader.shader.module, NULL);
  //delete[] shader.uniformSets;
}

uint32_t Wingine::get_format_element_size(WgAttribFormat format){
  return sizeof(float) * format;
}

VkFormat Wingine::get_vkformat(WgAttribFormat att){
  switch(att){
    case WG_ATTRIB_FORMAT_1:
      return VK_FORMAT_R32_SFLOAT;
    case WG_ATTRIB_FORMAT_2:
      return VK_FORMAT_R32G32_SFLOAT;
    case WG_ATTRIB_FORMAT_3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case WG_ATTRIB_FORMAT_4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
      printf("Did not recognize WgAttributeFormat\n");
      exit(-1);
  }
}

WinginePipeline Wingine::createPipeline(std::initializer_list<WingineResourceSetLayout> resourceLayouts,
					std::initializer_list<WingineShader> shaders,
					std::initializer_list<WgAttribFormat> attribFormats,
					bool clear, std::initializer_list<WgAttachmentType> attachmentTypes){
  return createPipeline(resourceLayouts.size(), std::begin(resourceLayouts),
			shaders.size(), std::begin(shaders),
			attribFormats.size(), std::begin(attribFormats),
			clear, attachmentTypes.size(), std::begin(attachmentTypes));
						  
}

WinginePipeline Wingine::createPipeline(int numResourceLayouts, const WingineResourceSetLayout* resourceLayouts,
					std::initializer_list<WingineShader> shaders,
					std::initializer_list<WgAttribFormat> attribFormats,
					bool clear){
  WgAttachmentType attch[] = {WG_ATTACHMENT_TYPE_COLOR, WG_ATTACHMENT_TYPE_DEPTH};
  return createPipeline(numResourceLayouts, resourceLayouts,
			shaders.size(), std::begin(shaders),
			attribFormats.size(), std::begin(attribFormats),
			clear, 2, attch);
}

WinginePipeline Wingine::createPipeline(int numResourceLayouts, const WingineResourceSetLayout* resourceLayouts,
					int numShaders, const WingineShader* shaders,
					int numVertexAttribs, const WgAttribFormat* attribFormats,
					bool clear, int numAttachments, const WgAttachmentType* attachmentTypes){
  WinginePipelineSetup pipelineSetup(numAttachments, attachmentTypes);
  WinginePipeline pipeline;
  pipeline.clearAttachments = clear;
  pipeline.numAttachments = numAttachments;
  wgAssert(numVertexAttribs <= MAX_VERTEX_ATTRIBUTES, "Max Vertex Attributes high enough");
  pipeline.numVertexAttribs = numVertexAttribs;
  pipeline_setup_generic(&pipelineSetup, numVertexAttribs);
  pipelineSetup.renderPassSetup.numAttachments = numAttachments;

  if(clear){
    for(int i = 0; i < numAttachments; i++){
      pipelineSetup.renderPassSetup.attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
  }

  VkPipelineShaderStageCreateInfo* mods = new VkPipelineShaderStageCreateInfo[numShaders];
  for(int i=  0; i < numShaders; i++){
    mods[i] = shaders[i].shader;
  }
  
  for(int i = 0; i < numVertexAttribs; i++){
    pipelineSetup.vi_bindings[i].binding = i;
    pipelineSetup.vi_bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    pipelineSetup.vi_bindings[i].stride = get_format_element_size(attribFormats[i]);
    
    pipelineSetup.vi_attribs[i].binding = i;
    pipelineSetup.vi_attribs[i].location = i;
    pipelineSetup.vi_attribs[i].format = get_vkformat(attribFormats[i]);
    pipelineSetup.vi_attribs[i].offset = 0;
  }

  pipelineSetup.createInfo.pStages = mods;
  pipelineSetup.createInfo.stageCount = numShaders;

  pipeline.numDescriptorSetLayouts = numResourceLayouts;
  pipeline.descriptorSetLayouts = new VkDescriptorSetLayout[numResourceLayouts];
  for(int i = 0; i < numResourceLayouts; i++){
    pipeline.descriptorSetLayouts[i] = resourceLayouts[i].layout;
  }

  pipelineSetup.layoutCreateInfo.setLayoutCount = numResourceLayouts;
  pipelineSetup.layoutCreateInfo.pSetLayouts = pipeline.descriptorSetLayouts;
  VkResult res = vkCreatePipelineLayout(device, &pipelineSetup.layoutCreateInfo, NULL, &pipeline.pipelineLayout);
  wgAssert(res == VK_SUCCESS, "Creating pipeline layout");

  res = vkCreateRenderPass(device, &pipelineSetup.renderPassSetup.createInfo, NULL, &pipeline.compatibleRenderPass);
  wgAssert(res == VK_SUCCESS, "Creating render pass");

  pipelineSetup.createInfo.layout =  pipeline.pipelineLayout;
  pipelineSetup.createInfo.renderPass = pipeline.compatibleRenderPass;

  res = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipelineSetup.createInfo, NULL, &pipeline.pipeline);
  wgAssert(res == VK_SUCCESS, "Creating pipeline");

  delete[] mods;

  return pipeline;
}

WinginePipeline Wingine::createPipeline(WingineResourceSetLayout resourceLayout,
  int numShaders, const WingineShader* shaders, int numVertexAttribs, const WgAttribFormat* attribFormats,
  bool clear, int numAttachments, const WgAttachmentType* attachmentTypes){
  return createPipeline(1, &resourceLayout, numShaders, shaders, numVertexAttribs, attribFormats,
			clear, numAttachments, attachmentTypes);
}

WinginePipeline Wingine::createPipeline(std::initializer_list<WingineResourceSetLayout> layouts,
					std::initializer_list<WingineShader> shaders,
					std::initializer_list<WgAttribFormat> attribTypes,
					bool clear){
  return createPipeline(layouts, shaders, attribTypes, clear, {WG_ATTACHMENT_TYPE_COLOR,
	WG_ATTACHMENT_TYPE_DEPTH});
}
WinginePipeline Wingine::createPipeline(WingineResourceSetLayout resourceLayout,
  int numShaders, const WingineShader* shaders, int numVertexAttribs, const WgAttribFormat* attribTypes,
  bool clear){
  WgAttachmentType attachTypes[2] = {WG_ATTACHMENT_TYPE_COLOR, WG_ATTACHMENT_TYPE_DEPTH};
  return createPipeline(resourceLayout, numShaders, shaders, numVertexAttribs,
    attribTypes, clear, 2, attachTypes);
}

WinginePipeline Wingine::createDepthPipeline(std::initializer_list<WingineResourceSetLayout> layouts,
				    std::initializer_list<WingineShader> shaders){
  return createDepthPipeline(layouts.size(), std::begin(layouts),
			     shaders.size(), std::begin(shaders));
}
WinginePipeline Wingine::createDepthPipeline(int numResourceSets, const WingineResourceSetLayout* resourceLayouts,
					     int numShaders, const WingineShader* shaders){
  WgAttachmentType attachmentType = WG_ATTACHMENT_TYPE_DEPTH;
  WgAttribFormat format = WG_ATTRIB_FORMAT_4;
  return createPipeline(numResourceSets, resourceLayouts, numShaders, shaders, 1, &format,
			true, 1, &attachmentType);
}

// Includes only one vertex attrib. Assumes that position is the only vertex attribute
WinginePipeline Wingine::createDepthPipeline(WingineResourceSetLayout resourceLayout,
					     int numShaders, const WingineShader* shaders){
  return createDepthPipeline(1, &resourceLayout, numShaders, shaders);
}

void Wingine::destroyPipeline(WinginePipeline pipeline){
  vkDestroyPipelineLayout(device, pipeline.pipelineLayout, NULL);
  vkDestroyRenderPass(device, pipeline.compatibleRenderPass, NULL);
  vkDestroyPipeline(device, pipeline.pipeline, NULL);
  delete[] pipeline.descriptorSetLayouts;
}

#ifdef WINGINE_WITH_GLSLANG
WingineKernel Wingine::createKernel(const char* kernelText, WingineResourceSetLayout resourceLayout){
  WingineKernel wk;

  wk.shader = createShader(kernelText, WG_SHADER_STAGE_COMPUTE);

  VkPipelineLayoutCreateInfo lCreateInfo = {};
  lCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  lCreateInfo.pNext = NULL;
  lCreateInfo.setLayoutCount = 1;
  lCreateInfo.pSetLayouts = &resourceLayout.layout;

  VkResult res = vkCreatePipelineLayout(device, &lCreateInfo, NULL, &wk.layout);

  wgAssert(res == VK_SUCCESS, "Create pipeline layout for kernel");

  VkComputePipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.layout = wk.layout;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.stage = wk.shader.shader;

  res = vkCreateComputePipelines(device,pipeline_cache,1,&pipelineCreateInfo,NULL,&wk.pipeline);

  return wk;
}
#endif // WINGINE_WITH_GLSLANG

void Wingine::destroyKernel(WingineKernel kernel){
  vkDestroyPipelineLayout(device, kernel.layout, NULL);
  destroyShader(kernel.shader);
  vkDestroyPipeline(device, kernel.pipeline,NULL);
}

void Wingine::executeKernel(WingineKernel& kernel, WingineResourceSet resourceSet, int numX, int numY, int numZ){
  VkResult res;
  do{
    res = vkWaitForFences(device, 1, &compute_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  res = vkResetFences(device, 1, &compute_command_buffer_fence);

  VkCommandBufferBeginInfo cmd_begin = {};
  cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin.pNext = NULL;

  vkBeginCommandBuffer(compute_command_buffer, &cmd_begin);

  vkCmdBindPipeline(compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, kernel.pipeline);
  vkCmdBindDescriptorSets(compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,kernel.layout, 0, 1, &resourceSet.descriptorSet, 0, NULL);
  vkCmdDispatch(compute_command_buffer, numX, numY, numZ);

  vkEndCommandBuffer(compute_command_buffer);

  VkSubmitInfo computeSubmit = {};
  computeSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  computeSubmit.pNext = NULL;
  computeSubmit.commandBufferCount = 1;
  computeSubmit.pCommandBuffers = &compute_command_buffer;
  res = vkQueueSubmit(compute_queue, 1, &computeSubmit, compute_command_buffer_fence);

  wgAssert(res == VK_SUCCESS, "Submit compute command");

  //Play it safe for now
  do{
    res = vkWaitForFences(device, 1, &compute_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);
}

void Wingine::wg_cmd_set_image_layout(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
  VkImageLayout oldLayout, VkImageLayout newLayout){
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

  VkPipelineStageFlags srcStages, destStages;

  switch (oldLayout) {
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    srcStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;

  case VK_IMAGE_LAYOUT_PREINITIALIZED:
    image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    srcStages = VK_PIPELINE_STAGE_HOST_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    srcStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;

  default:
    srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    break;
  }

  switch (newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    destStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    destStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    destStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    destStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    destStages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    break;

  default:
    destStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    break;
  }

  vkCmdPipelineBarrier(cmd, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
}

void Wingine::setLayout(WingineImage& wim, VkImageLayout newLayout){
  VkCommandBufferBeginInfo beg = {};
  beg.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beg.pNext = NULL;
  beg.flags = 0;
  beg.pInheritanceInfo = NULL;
  VkResult res;

  do{
    res = vkWaitForFences(device, 1, &free_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  wgAssert(res == VK_SUCCESS, "Waiting for free_command_buffer_fence");

  res = vkResetFences(device, 1, &free_command_buffer_fence);
  wgAssert(res == VK_SUCCESS, "Reset free command buffer");

  res = vkBeginCommandBuffer(free_command_buffer, &beg);

  wg_cmd_set_image_layout(free_command_buffer,wim.image,
    VK_IMAGE_ASPECT_COLOR_BIT, wim.layout, newLayout);

  res = vkEndCommandBuffer(free_command_buffer);
  wgAssert(res == VK_SUCCESS, "Record endCommandBuffer");

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
  wgAssert(res == VK_SUCCESS, "Submitting command buffer for changing image layout");

  wim.layout = newLayout;
  wim.imageInfo.imageLayout = newLayout;

  vkDestroyImageView(device, wim.view, NULL);

  // Playing it safe
  do{
    res = vkWaitForFences(device, 1, &free_command_buffer_fence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.image = wim.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  res = vkCreateImageView(device,&viewInfo, NULL, &wim.view);

  wim.imageInfo.imageView = wim.view;
}

void Wingine::copyImage(int w, int h, VkImage srcImage, VkImageLayout srcStartLayout, VkImageLayout srcEndLayout,
			int w2, int h2, VkImage dstImage, VkImageLayout dstStartLayout, VkImageLayout dstEndLayout,
			VkImageAspectFlagBits aspect){
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
  wg_cmd_set_image_layout(free_command_buffer, srcImage, aspect, srcStartLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  wg_cmd_set_image_layout(free_command_buffer, dstImage, aspect, dstStartLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Transfer

  if(aspect == VK_IMAGE_ASPECT_DEPTH_BIT){
    if(w != w2 || h != h2){
      printf("Cannot copy depth image to a differently sized image");
      exit(-1);
    }
    VkImageCopy copy;
    copy.srcSubresource.aspectMask = aspect;
    copy.srcSubresource.mipLevel = 0;
    copy.srcSubresource.baseArrayLayer = 0;
    copy.srcSubresource.layerCount = 1;
    copy.srcOffset.x = 0;
    copy.srcOffset.y = 0;
    copy.srcOffset.z = 0;
    copy.dstSubresource.aspectMask = aspect;
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
  }else{
    VkImageBlit region = {};
    region.srcSubresource.aspectMask = aspect; //VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffsets[0].x = 0;
    region.srcOffsets[0].y = 0;
    region.srcOffsets[1].x = w;
    region.srcOffsets[1].y = h;
    region.srcOffsets[1].z = 1;

    region.dstSubresource.aspectMask = aspect; //VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstOffsets[0].x = 0;
    region.dstOffsets[0].y = 0;
    region.dstOffsets[1].x = w2;
    region.dstOffsets[1].y = h2;
    region.dstOffsets[1].z = 1;

    VkFilter filter = aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) ?
      VK_FILTER_NEAREST : VK_FILTER_LINEAR;

    vkCmdBlitImage(free_command_buffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);
  }


  // Set the layouts of the images for use later
  if(srcEndLayout != VK_IMAGE_LAYOUT_UNDEFINED){
    wg_cmd_set_image_layout(free_command_buffer, srcImage, aspect, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcEndLayout);
  }

  wg_cmd_set_image_layout(free_command_buffer, dstImage, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstEndLayout);

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

void Wingine::copyColorImage(WingineImage& src, WingineImage& dst){

  VkImageLayout srcEndLayout = src.goalLayout;
  VkImageLayout dstEndLayout = dst.goalLayout;

  if(srcEndLayout == VK_IMAGE_LAYOUT_UNDEFINED || srcEndLayout == VK_IMAGE_LAYOUT_PREINITIALIZED){
    srcEndLayout = VK_IMAGE_LAYOUT_GENERAL;
  }

  if(dstEndLayout == VK_IMAGE_LAYOUT_UNDEFINED || dstEndLayout == VK_IMAGE_LAYOUT_PREINITIALIZED){
    dstEndLayout = VK_IMAGE_LAYOUT_GENERAL;
  }
  copyImage(src.width, src.height, src.image, src.layout, srcEndLayout,
	    dst.width, dst.height, dst.image, dst.layout, dstEndLayout, VK_IMAGE_ASPECT_COLOR_BIT);
  src.layout = srcEndLayout;
  dst.layout = dstEndLayout;
}

void Wingine::copyDepthImage(WingineImage& src, WingineImage& dst){

  VkImageLayout srcEndLayout;
  VkImageLayout dstEndLayout;

  if (src.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
    srcEndLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  } else if(src.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    srcEndLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  } else {
    printf("[Wingine] Error: Expected depth copy source to have either attachment usage or read only usage. Had usage 0x%x\n", src.usage);
    exit(-1);
  }

  if (dst.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
    dstEndLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  } else if(dst.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    dstEndLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  } else {
    printf("[Wingine] Error: Expected depth copy destination to have either attachment usage or read only usage. had usage 0x%x\n", dst.usage);
    exit(-1);
  }
  
  copyImage(src.width, src.height, src.image, src.layout, srcEndLayout,
	    dst.width, dst.height, dst.image, dst.layout, dstEndLayout, VK_IMAGE_ASPECT_DEPTH_BIT);
  src.layout = srcEndLayout;
  dst.layout = dstEndLayout;
}

void Wingine::copyColorFromFramebuffer(WingineFramebuffer src, WingineImage dst){
  copyColorImage(src.image, dst);
}

void Wingine::copyDepthFromFramebuffer(WingineFramebuffer src, WingineImage dst){
  copyDepthImage(src.depth_image, dst);
}

WingineImage Wingine::createImage(uint32_t w, uint32_t h, VkImageLayout layout, VkImageUsageFlags usage,
  VkImageTiling tiling, uint32_t memProps){
  WingineImage image;
  image.width = w;
  image.height = h;
  image.layout = layout;

  VkImage vim;

  VkImageCreateInfo ici = {};

  image_create_info_generic(&ici);
  ici.extent.width = w;
  ici.extent.height = h;
  ici.tiling = tiling;
  ici.initialLayout = layout;
  ici.usage = usage;

  VkResult res = vkCreateImage(device, &ici, NULL, &vim);
  wgAssert(res == VK_SUCCESS, "Create image");

  return create_image_from_vk_image(vim, w, h, usage, memProps);
}

WingineImage Wingine::createGPUImage(uint32_t w, uint32_t h){
  return createImage(w, h, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
}

WingineImage Wingine::create_image_from_vk_image(VkImage vkim, uint32_t w, uint32_t h, VkImageUsageFlags usage, uint32_t memProps){
  WingineImage image;
  image.width = w;
  image.height = h;
  image.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  image.image = vkim;

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.pNext = NULL;
  memAlloc.allocationSize = 0;
  memAlloc.memoryTypeIndex = 0;

  VkMemoryRequirements memReqs;

  vkGetImageMemoryRequirements(device, image.image, &memReqs);

  memAlloc.allocationSize = memReqs.size;
  image.memorySize = (int)memReqs.size;

  memAlloc.memoryTypeIndex = get_memory_type_index(memReqs.memoryTypeBits,
						   memProps);

  VkResult res = vkAllocateMemory(device, &memAlloc, NULL, &image.mem);
  wgAssert(res == VK_SUCCESS, "Allocate image memory");

  res = vkBindImageMemory(device, image.image, image.mem, 0);
  wgAssert(res == VK_SUCCESS, "Bind image memory");

  image.imageInfo.imageLayout = image.layout;

  if((usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) == usage){
    image.view = 0;
    return image;
  }

  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.image = image.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
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

  image.imageInfo.imageView = image.view;
  return image;
}

void Wingine::destroyImage(WingineImage w){
  if(w.view != 0)
    vkDestroyImageView(device, w.view, NULL);
  vkDestroyImage(device, w.image, NULL);
  vkFreeMemory(device, w.mem, NULL);
}

WingineFramebuffer Wingine::createDepthFramebuffer(uint32_t width, uint32_t height, WinginePipeline& pipeline){
  WingineFramebuffer framebuffer;
  framebuffer.depth_image = createDepthBuffer(width, height);


  VkImageView attachments[] = {framebuffer.depth_image.view};
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = pipeline.compatibleRenderPass;
  fb_info.attachmentCount = 1;
  fb_info.pAttachments = attachments;
  fb_info.width = width;
  fb_info.height = height;
  fb_info.layers = 1;

  VkResult res = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffer.framebuffer);
  wgAssert(res == VK_SUCCESS, "Creating framebuffer");

  framebuffer.image.image = 0;

  return framebuffer;
}

WingineFramebuffer Wingine::createFramebuffer(uint32_t width, uint32_t height){
  VkImageCreateInfo ici = {};

  image_create_info_generic(&ici);
  ici.extent.width = width;
  ici.extent.height = height;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  VkImage vim;
  VkResult res = vkCreateImage(device, &ici, NULL, &vim);
  wgAssert(res == VK_SUCCESS, "Create image");

  return create_framebuffer_from_vk_image(vim, width, height, false);
}

WingineFramebuffer Wingine::create_framebuffer_from_vk_image(VkImage vim, uint32_t width, uint32_t height,
        bool hasMemory){
  WingineImage colIm;
  if(!hasMemory){
    colIm = create_image_from_vk_image(vim, width, height,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  }else{
    colIm.width = width;
    colIm.height = height;
    colIm.image = vim;
    VkImageViewCreateInfo color_image_view = {};

    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = NULL;
    color_image_view.flags = 0;
    color_image_view.image = vim;
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

    VkResult res = vkCreateImageView(device, &color_image_view, NULL, &colIm.view);
    wgAssert(res == VK_SUCCESS, "Creating image view for vk_image");
  }


  colIm.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  colIm.imageInfo.imageView = colIm.view;
  colIm.imageInfo.imageLayout = colIm.layout;

  WingineFramebuffer framebuffer;
  framebuffer.image = colIm;
  framebuffer.depth_image = createDepthBuffer(width, height);

  VkImageView attachments[] = {colIm.view, framebuffer.depth_image.view};
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = render_pass_generic;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = attachments;
  fb_info.width = width;
  fb_info.height = height;
  fb_info.layers = 1;
  VkResult res = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffer.framebuffer);
  wgAssert(res == VK_SUCCESS, "Creating framebuffer");
  return framebuffer;
}

// The usage flags are a bit of a chaos
WingineImage Wingine::createReadableDepthBuffer(uint32_t width, uint32_t height){
  return createDepthBuffer(width, height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
}

WingineImage Wingine::createDepthBuffer(uint32_t width, uint32_t height){
  return createDepthBuffer(width, height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
}

WingineImage Wingine::createDepthBuffer(uint32_t width, uint32_t height, uint32_t usage){
  WingineImage depth_buffer;

  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(physical_device, DEPTH_BUFFER_FORMAT, &props);


  VkImageCreateInfo image_info = {};
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
  image_info.format = DEPTH_BUFFER_FORMAT;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = NUM_SAMPLES;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = NULL;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.flags = 0;

  VkMemoryAllocateInfo mem_alloc = {};
  mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_alloc.pNext = NULL;
  mem_alloc.allocationSize = 0;
  mem_alloc.memoryTypeIndex = 0;



  VkResult res = vkCreateImage(device, &image_info, NULL, &depth_buffer.image);
  if(res != VK_SUCCESS){
    printf("Could not create depth buffer\n");
    exit(0);
  }

  VkMemoryRequirements mem_reqs = {};
  vkGetImageMemoryRequirements(device, depth_buffer.image, &mem_reqs);
  mem_alloc.allocationSize = mem_reqs.size;

  mem_alloc.memoryTypeIndex = get_memory_type_index(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  res = vkAllocateMemory(device, &mem_alloc, NULL, &depth_buffer.mem);
  if(res != VK_SUCCESS){
    printf("Memory could not be allocated\n");
    exit(0);
  }

  res = vkBindImageMemory(device, depth_buffer.image, depth_buffer.mem, 0);
  if(res != VK_SUCCESS){
    printf("Memory could not be bound\n");
    exit(0);
  }

  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = NULL;
  view_info.image = depth_buffer.image;
  view_info.format = DEPTH_BUFFER_FORMAT;
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

  res = vkCreateImageView(device, &view_info, NULL, &depth_buffer.view);
  if(res != VK_SUCCESS){
    printf("Depth image view could not be created\n");
    exit(0);
  }

  depth_buffer.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_buffer.width = width;
  depth_buffer.height = height;
  depth_buffer.usage = usage;

  depth_buffer.imageInfo.imageLayout = depth_buffer.layout;
  depth_buffer.imageInfo.imageView = depth_buffer.view;
  
  return depth_buffer;
}

WingineImage Wingine::create_mappable_image(uint32_t width, uint32_t height){
  return createImage(width, height, VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_IMAGE_TILING_LINEAR,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  );
}

void Wingine::getImageContent(WingineImage image, uint8_t* data){
  WingineImage mapIm = create_mappable_image(image.width, image.height);

  copyColorImage(image, mapIm);

  VkImageSubresource subres = {};
  subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subres.mipLevel = 0;
  subres.arrayLayer = 0;
  VkSubresourceLayout srLayout;
  vkGetImageSubresourceLayout(device, mapIm.image, &subres, &srLayout);

  uint8_t* tmp;
  VkResult res = vkMapMemory(device, mapIm.mem, 0, mapIm.memorySize, 0, (void**)&tmp);
  wgAssert(res == VK_SUCCESS, "Mapping memory in image content getting");
  tmp += srLayout.offset;
  uint32_t* dataRow = (uint32_t*)data;
  for(uint32_t i = 0; i < mapIm.height; i++){
    uint32_t*tp = (uint32_t*)tmp;
    for(uint32_t j = 0; j < mapIm.width; j++){
      *dataRow = *tp;
      tp++;
      dataRow++;
    }
    //memcpy(dataRow, tmp, mapIm.width*4);
    tmp += srLayout.rowPitch;
    //dataRow += mapIm.width * 4;
  }

  vkUnmapMemory(device, mapIm.mem);
  destroyImage(mapIm);
}

WingineTexture Wingine::createDepthTexture(int w, int h){
  VkResult res;

  WingineTexture resultTexture;

  resultTexture.image.width = w;
  resultTexture.image.height = h;

  //Now, create the actual image
  resultTexture.image = createDepthBuffer(w,h, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
					  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
					  VK_IMAGE_USAGE_SAMPLED_BIT);

  //Initialize sampler
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1;
  samplerInfo.compareOp = VK_COMPARE_OP_LESS;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 1.0f;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  res = vkCreateSampler(device, &samplerInfo, NULL, &resultTexture.sampler);
  wgAssert(res == VK_SUCCESS, "Creating image sampler");

  resultTexture.image.imageInfo.imageView = resultTexture.image.view;
  resultTexture.image.imageInfo.sampler = resultTexture.sampler;
  
  resultTexture.image.imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  resultTexture.image.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  resultTexture.image.goalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  return resultTexture;
}

//This will create a texture immutable from the CPU
WingineTexture Wingine::createTexture(int w, int h, unsigned char* imageBuffer)
{
  VkResult res;

  WingineTexture resultTexture;

  resultTexture.image.width = w;
  resultTexture.image.height = h;


  WingineImage wMappableImage = create_mappable_image(w, h);
  VkImage mappableImage = wMappableImage.image;

  VkImageSubresource subres = {};
  subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subres.mipLevel = 0;
  subres.arrayLayer = 0;

  VkSubresourceLayout layout;
  void *data;

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(device, mappableImage, &memReqs);

  vkGetImageSubresourceLayout(device, mappableImage, &subres, &layout);

  res = vkMapMemory(device, wMappableImage.mem, 0,memReqs.size, 0, &data);
  wgAssert(res == VK_SUCCESS, "Map memory");

  unsigned char* currRow = (unsigned char*)data;
  for(int i = 0; i < h; i++){
    memcpy(currRow, &imageBuffer[4*i*w], w*4);
    currRow += layout.rowPitch;
  }

  vkUnmapMemory(device, wMappableImage.mem);

  //Now, create the actual image
  resultTexture.image = createImage(w,h,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);


  copyColorImage(wMappableImage, resultTexture.image);

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

  destroyImage(wMappableImage);

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
  res = vkQueueWaitIdle(graphics_queue); // This is a bit of a show stopper
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

WingineFramebuffer Wingine::getCurrentFramebuffer() const {
  return framebuffers[current_buffer];
}

WingineFramebuffer Wingine::getLastFramebuffer() const {
  return framebuffers[(current_buffer - 1 + swapchain_image_count) % swapchain_image_count];
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


WingineRenderObject::WingineRenderObject(int numInds, std::initializer_list<WingineBuffer> vertexBuffers, const WingineBuffer& iBuffer) : WingineRenderObject(numInds, vertexBuffers.size(), std::begin(vertexBuffers), iBuffer) {
}

WingineRenderObject::WingineRenderObject(int numInds, int numVertexAttribs, const WingineBuffer* buffers, const WingineBuffer& iBuffer){
  for(int i = 0; i < numVertexAttribs; i++){
    vertexAttribs.push_back(buffers[i]);
  }
  indexBuffer = iBuffer;

  numDrawIndices = numInds;
  indexOffset = 0;
}

void WingineRenderObject::destroy(){
}

void Wingine::destroyObject(WingineRenderObject& obj){
  destroyBuffer(obj.getIndexBuffer());
  for(uint32_t i = 0; i < obj.getNumAttribs(); i++) {
    destroyBuffer(obj.getAttribs()[i]);
  }
  obj.destroy();
}

void WingineRenderObject::setIndexOffset(int newIndex){
  indexOffset = newIndex;
}

void WingineRenderObject::setNumIndices(int num){
  numDrawIndices = num;
}

void WingineRenderObject::setPipeline(const WinginePipeline& p){
  pipeline = &p;
}


// Not even sure why these were here
/* void WingineRenderObject::setIndexBuffer(const WingineBuffer& ib){
  indexBuffer = ib;
  altered = true;
}

void WingineRenderObject::setVertexAttribs(const WingineBuffer& wb, int index){
  vertexAttribs[index] = wb;
  altered = true;
  } */

uint32_t WingineRenderObject::getNumIndices() const {
  return numDrawIndices;
}

uint32_t WingineRenderObject::getIndexOffset() const {
  return indexOffset;
}

const WingineBuffer& WingineRenderObject::getIndexBuffer() const {
  return indexBuffer;
}

uint32_t WingineRenderObject::getNumAttribs() const {
  return vertexAttribs.size();
}

const WingineBuffer* WingineRenderObject::getAttribs() const {
  return vertexAttribs.data();
}

void WingineRenderObject::addVertexAttrib(const WingineBuffer& buffer) {
  vertexAttribs.push_back(buffer);
}

void WingineObjectGroup::recordRendering(WingineRenderObject& object,
					 std::initializer_list<WingineResourceSet> sets){
  recordRendering(object, std::begin(sets));
}

void WingineObjectGroup::recordRendering(WingineRenderObject& object,
					 const WingineResourceSet* resourceSets){
  wgAssert(MAX_DESCRIPTOR_SETS >= pipeline.numDescriptorSetLayouts, "Enough descriptor sets");
  VkDescriptorSet sets[MAX_DESCRIPTOR_SETS];
  for(int i = 0; i < pipeline.numDescriptorSetLayouts; i++){
    sets[i] = resourceSets[i].descriptorSet;
  }

  if(pipeline.numDescriptorSetLayouts){
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout,
			    0, pipeline.numDescriptorSetLayouts, sets, 0, NULL);
  }

  VkBuffer vertexBuffers[MAX_VERTEX_ATTRIBUTES];
  VkDeviceSize offsets[MAX_VERTEX_ATTRIBUTES];
  uint32_t numAttribs = pipeline.numVertexAttribs;
  const WingineBuffer* attribs = object.getAttribs();
  for(uint32_t i = 0; i < numAttribs; i++){
    vertexBuffers[i] = attribs[i].buffer;
    offsets[i] = 0;
  }

  vkCmdBindVertexBuffers(commandBuffer, 0, numAttribs, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, object.getIndexBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(commandBuffer, object.getNumIndices(), 1, object.getIndexOffset(), 0, 0);

  altered = false;
}

void WingineRenderObject::setObjectGroup(WingineObjectGroup& wog){
  objectGroup = &wog;
}

bool WingineRenderObject::isAltered(){
  return altered;
}

void WingineObjectGroup::addObject(const WingineRenderObject& obj){
  objects.push_back(obj);
  int ind = objects.size() - 1;
  objects[ind].setPipeline(pipeline);
  altered = true;
  objects[ind].setObjectGroup(*this);
}

WingineObjectGroup::WingineObjectGroup(Wingine& wg, const WinginePipeline& newPipeline) : wingine(&wg), pipeline(newPipeline) {
  for(int i = 0; i < 3; i++) {
    clearColor[i] = 0.0f;
  }
  clearColor[3] = 1.0f;
  
  shouldClearAttachments = newPipeline.clearAttachments;
  commandBuffer = wg.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

void WingineObjectGroup::setClearColor(float r, float g, float b, float alpha) {
  clearColor[0] = r;
  clearColor[1] = g;
  clearColor[2] = b;
  clearColor[3] = alpha;
}

const float* WingineObjectGroup::getClearColor() {
  return clearColor;
}

void WingineObjectGroup::startRecording(){
  startRecording(wingine->getCurrentFramebuffer());
}

void WingineObjectGroup::startRecording(const WingineFramebuffer& framebuffer){
  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo begin = {};
  begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin.pNext = NULL;
  begin.flags = 0;
  begin.pInheritanceInfo = NULL;

  std::vector<VkClearValue> clear_values(pipeline.numAttachments);

  int framebuffer_width, framebuffer_height;

  if(framebuffer.image.image){
    framebuffer_width = framebuffer.image.width;
    framebuffer_height = framebuffer.image.height;

    for(uint32_t i = 0; i < 4; i++) {
      clear_values[0].color.float32[i] = clearColor[i];
    }
    
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;
  }else{
    framebuffer_width = framebuffer.depth_image.width;
    framebuffer_height = framebuffer.depth_image.height;

    clear_values[0].depthStencil.depth = 1.0f;
  }
  
  VkViewport viewport;
  viewport.width = (float)framebuffer_width;
  viewport.height = (float)framebuffer_height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  viewport.x = 0;
  viewport.y = 0;

  VkRect2D scissor;
  scissor.extent.width = framebuffer_width;
  scissor.extent.height = framebuffer_height;
  scissor.offset.x = 0;
  scissor.offset.y = 0;

  VkRenderPassBeginInfo rp_begin = {};
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = NULL;
  rp_begin.renderPass = pipeline.compatibleRenderPass;
  rp_begin.clearValueCount = shouldClearAttachments? pipeline.numAttachments : 0;

  rp_begin.pClearValues = clear_values.data();
  rp_begin.framebuffer = framebuffer.framebuffer;
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = framebuffer_width;
  rp_begin.renderArea.extent.height = framebuffer_height;

  VkResult res = vkBeginCommandBuffer(commandBuffer, &begin);
  wgAssert(res == VK_SUCCESS, "Begin command buffer");

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

  vkCmdBeginRenderPass(commandBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
}


void WingineObjectGroup::endRecording(){
  vkCmdEndRenderPass(commandBuffer);
  VkResult res;
  res = vkEndCommandBuffer(commandBuffer);
  wgAssert(res == VK_SUCCESS, "End command buffer");

  wingine->submitDrawCommandBuffer(commandBuffer);
}

WingineRenderObject::WingineRenderObject(){
}

void* _place_in_heap(void * data, int size){
  void* pp = malloc(size);
  memcpy(pp, data, size);
  return pp;
}


namespace wgutil {
  static WingineResourceSetLayout transformSetLayout; // To avoid duplicating the layout
  static bool rslInitiated = false;
  static int objCount = 0; // To delete layout when number of objects is zero

  void Model::initModel(Wingine& wg){
    wingine = &wg;

    if(!rslInitiated){
      transformSetLayout = wg.createResourceSetLayout({WG_RESOURCE_TYPE_UNIFORM}, {WG_SHADER_STAGE_VERTEX});
      rslInitiated = true;
    }

    objCount++;

    transformUniform = wg.createUniform(sizeof(Mat4));
    resourceSet = wg.createResourceSet(transformSetLayout);
    wg.updateResourceSet(resourceSet, {&transformUniform});

    wingine->setUniform(transformUniform, &transform, sizeof(Mat4));
  }
  
  Model::Model(Wingine& wg, WgModelInitMode mode,
	       const char* file_name, std::initializer_list<WgAttribType> attribs){
    initModel(wg);

    switch(mode){
    case WG_MODEL_INIT_READ_OBJ:
      initFromFile(file_name, attribs);
      break;
    default:
      printf("No matching call to Model() with mode %d\n", mode);
      exit(-1);
    }

  }

  Model::Model(Wingine& wg, WgModelInitMode mode, std::initializer_list<uint32_t> sizes,
	       std::initializer_list<void (*)(float,float, float*)> generators,
	       int numT, int numH, float t10, float t11){
    initModel(wg);
    switch(mode){
    case WG_MODEL_INIT_POLY:
      initPolyhedron(sizes, generators, numT, numH, t10, t11);
      break;
    default:
      printf("No matching call to Model() with mode %d\n", mode);
      exit(-1);
    }
  }

  Model::Model(Wingine& wg, WgModelInitMode mode, std::initializer_list<WgAttribType> attribs, float size){
    initModel(wg);

    switch(mode){
    case WG_MODEL_INIT_CUBE:
      initCube(attribs, size);
      break;
    default:
      printf("No matching call to Model() with mode %d\n", mode);
      exit(-1);
    }
  }

  Model::Model(Wingine& wg, WgModelInitMode mode, std::initializer_list<WgAttribType> attribs, float t0, int t1) {
    initModel(wg);
    switch(mode) {
    case WG_MODEL_INIT_SPHERE:
      initSphere(attribs, t0, t1);
      break;
    default:
      printf("No matching call to Model() with mode %d\n", mode);
    }
  }
  
  Model::Model(Wingine& wg, WgModelInitMode mode, std::initializer_list<WgAttribType> attribs,
	       const Vec3& v2, const Vec3& v3){
    initModel(wg);
    switch(mode){
    case WG_MODEL_INIT_QUAD:
      initQuad(attribs, v2, v3);
      break;
    default:
      printf("No matching call to Model() with mode %d\n", mode);
      exit(-1);
    }
  }

  Model::Model(Wingine& wg, int numInds, int numVertexAttribs,
	       const WingineBuffer* vertexBuffers, 
	       const WingineBuffer& indBuffer) {
    initModel(wg);

    for(int i = 0; i < numVertexAttribs; i++) {
      vertexAttribs.push_back(vertexBuffers[i]);
    }
    
    indexBuffer = indBuffer;
    numDrawIndices = numInds;
    indexOffset = 0;
    
  }

  // Input from file. Only supports one set of values per attrib type
  void Model::initFromFile(const char* file_name, std::initializer_list<WgAttribType> attribs){
    int attribIndices[WG_NUM_ATTRIB_TYPES];
    const WgAttribType* arr = std::begin(attribs);
    
    for(uint32_t i = 0; i < attribs.size(); i++){
      if(arr[i] == WG_ATTRIB_TYPE_COLOR){
	printf("Color nodes from .obj-files are not yet supported\n");
	exit(-1);
      }
    }
    
    for(int j = 0; j < WG_NUM_ATTRIB_TYPES; j++){
      attribIndices[j] = -1;
      for(uint32_t i = 0; i < attribs.size(); i++){
	if(arr[i] == j){
	  attribIndices[j] = i;
	  break;
	}
      }
    }

    std::ifstream file(file_name);
    std::vector<std::vector<float> > data(attribs.size());

    std::vector<uint32_t> indices;
    std::string line;
    std::string type;

    float a, b, c;
    uint32_t u, v, w;

    const float scale = 1.f/50;
    const float up = 1.f;

    while(std::getline(file, line)){
      std::istringstream iss(line);
      iss >> type;
      if(attribIndices[WG_ATTRIB_TYPE_POSITION] != -1 && type == "v"){
	iss >> a >> b >> c;
	std::vector<float>& vertices = data[attribIndices[WG_ATTRIB_TYPE_POSITION]];
	vertices.push_back(a*scale);
	vertices.push_back(b*scale + up);
	vertices.push_back(c*scale);
	vertices.push_back(1.0f);
      } else if(attribIndices[WG_ATTRIB_TYPE_NORMAL] != -1 && type == "vn"){
	iss >> a >> b >> c;
	std::vector<float>& normals = data[attribIndices[WG_ATTRIB_TYPE_NORMAL]];
	normals.push_back(a);
	normals.push_back(b);
	normals.push_back(c);
	normals.push_back(0.0f);
      } else if(attribIndices[WG_ATTRIB_TYPE_TEXTURE] != -1 && type == "vt"){
        iss >> a >> b;
	std::vector<float>& coords = data[attribIndices[WG_ATTRIB_TYPE_TEXTURE]];
	coords.push_back(a);
	coords.push_back(b);
      }else if(type == "f"){
	iss >> u >> v >> w;
	indices.push_back(u-1);
	indices.push_back(v-1);
	indices.push_back(w-1);
      } else {
#ifdef DEBUG
	printf("Skipping line '%s', as its type is unknown\n", line.c_str());
#endif
      }
    }

    for(uint32_t i = 0; i < attribs.size(); i++){
      WingineBuffer buffer = wingine->createVertexBuffer(data[i].size() * sizeof(float), data[i].data());
      vertexAttribs.push_back(buffer);
    }
    WingineBuffer indBuffer = wingine->createIndexBuffer(indices.size() * sizeof(int32_t), indices.data());

    indexBuffer = indBuffer;
    numDrawIndices = indices.size();
    indexOffset = 0;
  }

  void Model::destroy(){
    destroyKeepBuffers();
    
    for(uint32_t i = 0; i < vertexAttribs.size(); i++){
      wingine->destroyBuffer(vertexAttribs[i]);
    }

    wingine->destroyBuffer(indexBuffer);
  }

  void Model::destroyKeepBuffers() {
    wingine->destroyResourceSet(resourceSet);
    wingine->destroyUniform(transformUniform);
    
    objCount--;
    if(objCount == 0){
      wingine->destroyResourceSetLayout(transformSetLayout);
    }
  }

  WingineResourceSet& Model::getTransformSet(){
    return resourceSet;
  }

  void Model::setTransform(const Mat4& mat){
    transform = mat;
    Mat4 tmp = ~transform;
    wingine->setUniform(transformUniform, &tmp, sizeof(Mat4));
  }

  void Model::initPolyhedron(std::initializer_list<uint32_t> sizes,
			     std::initializer_list<void (*)(float,float, float*)> generators,
			     int numT, int numH, float t10, float t11){
    float stepT = t10 / numT;
    float stepH = t11 / numH;

    wgAssert(sizes.size() == generators.size(), "Equal argument sizes in initPolyhedron");
    const uint32_t * sizeArray = std::begin(sizes);

    void (* const* generatorList)(float, float, float*) = std::begin(generators);

    for(uint32_t k = 0; k < generators.size(); k++){
      std::vector<float> data(numT * numH * sizeArray[k]);
      uint32_t currInd = 0;
      float currStepH = 0.0f;
      for(int i = 0; i < numH; i++){
	        float currStepT = 0.0f;
	        for(int j = 0; j < numT; j++){
	             generatorList[k](currStepT, currStepH, data.data() + currInd);
	             currInd += sizeArray[k];

	             currStepT += stepT;
	        }
	        currStepH += stepH;
      }
      vertexAttribs.push_back(wingine->createVertexBuffer(numT * numH * sizeArray[k] * sizeof(float), data.data()));
    }

    numDrawIndices = 3 * (2 * numT * (numH - 1) + 2 * (numT - 2));
    indexOffset = 0;

    std::vector<uint32_t> indices(numDrawIndices);
    for(int i = 0; i < numH - 1; i++){
      for(int j = 0; j < numT; j++){
	       indices[3 * 2 * (numT * i + j) + 0] = (i + 1) * numT + j;
	       indices[3 * 2 * (numT * i + j) + 1] = i * numT + ((j + 1) % numT);
	       indices[3 * 2 * (numT * i + j) + 2] = i * numT + j;

         indices[3 * 2 * (numT * i + j) + 3] = (i + 1) * numT + ((j + 1) % numT);
         indices[3 * 2 * (numT * i + j) + 4] = i * numT + ((j + 1) % numT);
         indices[3 * 2 * (numT * i + j) + 5] = (i + 1) * numT + j;
      }
    }

    for(int i = 0; i < numT - 2; i++){
      indices[3 * 2 * numT * (numH - 1) + 3 * i + 0] = 0;
      indices[3 * 2 * numT * (numH - 1) + 3 * i + 1] = i + 1;
      indices[3 * 2 * numT * (numH - 1) + 3 * i + 2] = i + 2;

      indices[3 * 2 * numT * (numH - 1) + 3 * (numT - 2) + 0] = numT * (numH - 1) + i + 1;
      indices[3 * 2 * numT * (numH - 1) + 3 * (numT - 2) + 1] = numT * (numH - 1) + i;
      indices[3 * 2 * numT * (numH - 1) + 3 * (numT - 2) + 2] = numT * (numH - 1);
    }

    indexBuffer = wingine->createIndexBuffer(numDrawIndices * sizeof(uint32_t), indices.data());
  }

  void Model::initSphere(std::initializer_list<WgAttribType> attribs_in, float radius, int resolution) {
    const WgAttribType * attribs = std::begin(attribs_in);

    for(uint32_t i = 0;  i < attribs_in.size(); i++) {
      int attrib_size = WG_ATTRIB_TYPE_SIZE[attribs[i]];

      std::vector<float> data(attrib_size * (resolution * 2 * (resolution - 1) + 2));

      float invres = 1.f / resolution;
      
      if (attribs[i] == WG_ATTRIB_TYPE_POSITION ) {
	for(int i = 0; i < resolution - 1; i++) {
	  float phi = F_PI * (invres * (i + 1) - 1.f / 2);
	  float cphi = cosf(phi);
	  float sphi = sinf(phi);
	  for(int j = 0; j < resolution * 2; j++) {
	    float theta = F_PI * (invres * j);
	    float ctheta = cosf(theta);
	    float stheta = sinf(theta);
	    
	    data[attrib_size * (i * resolution * 2 + j) + 0] = cphi * ctheta * radius;
	    data[attrib_size * (i * resolution * 2 + j) + 1] = sphi * radius;
	    data[attrib_size * (i * resolution * 2 + j) + 2] = cphi * stheta * radius;
	    data[attrib_size * (i * resolution * 2 + j) + 3] = 1.f;
	  }
	}

	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 0] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 1] = - radius;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 2] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 3] = 1.f;

	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 0] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 1] = radius;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 2] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 3] = 1.f;
	
      } else if (attribs[i] == WG_ATTRIB_TYPE_NORMAL ) {
	for(int i = 0; i < resolution - 1; i++) {
	  float phi = F_PI * (invres * (i + 1) - 1.f / 2);
	  float cphi = cosf(phi);
	  float sphi = sinf(phi);
	  for(int j = 0; j < resolution * 2; j++) {
	    float theta = F_PI * (invres * j);
	    float ctheta = cosf(theta);
	    float stheta = sinf(theta);

	    data[attrib_size * (i * resolution * 2 + j) + 0] = cphi * ctheta;
	    data[attrib_size * (i * resolution * 2 + j) + 1] = sphi;
	    data[attrib_size * (i * resolution * 2 + j) + 2] = cphi * stheta;
	    data[attrib_size * (i * resolution * 2 + j) + 3] = 0.f;
	  }
	}
	
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 0] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 1] = -1.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 2] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 3] = 1.f;

	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 0] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 1] = 1.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 2] = 0.f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 3] = 1.f;
	
      } else if (attribs[i] == WG_ATTRIB_TYPE_TEXTURE ) {
	for(int i = 0; i < resolution - 1; i++) {
	  float phi = invres * (i + 1);
	  for(int j = 0; j < resolution * 2; j++) {
	    float theta = invres * j;

	    data[attrib_size * (i * resolution * 2 + j) + 0] = theta;
	    data[attrib_size * (i * resolution * 2 + j) + 1] = phi;
	  }
	}

	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 0] = 0.5f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 0) + 1] = 1.f;
	
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 0] = 0.5f;
	data[attrib_size * (resolution * 2 * (resolution - 1) + 1) + 1] = 0.f;
      }
      
      vertexAttribs.push_back(wingine->createVertexBuffer(attrib_size * (resolution * 2 * (resolution - 1) + 2) * sizeof(float), data.data()));
    }

    numDrawIndices = 6 * resolution * 2 * (resolution - 2) + 2 * 3 * 2 * resolution;
    indexOffset = 0;

    std::vector<uint32_t> indices(numDrawIndices);

    for(int i = 0; i < resolution - 2; i++){
      for(int j = 0; j < resolution * 2; j++) {
	indices[6 * (resolution * 2 * i + j) + 0] = (i + 0) * 2 * resolution + ((j + 0) % (2 * resolution));
	indices[6 * (resolution * 2 * i + j) + 1] = (i + 0) * 2 * resolution + ((j + 1) % (2 * resolution));
	indices[6 * (resolution * 2 * i + j) + 2] = (i + 1) * 2 * resolution + ((j + 0) % (2 * resolution));

	indices[6 * (resolution * 2 * i + j) + 3] = (i + 1) * 2 * resolution + ((j + 0) % (2 * resolution));
	indices[6 * (resolution * 2 * i + j) + 4] = (i + 0) * 2 * resolution + ((j + 1) % (2 * resolution));
	indices[6 * (resolution * 2 * i + j) + 5] = (i + 1) * 2 * resolution + ((j + 1) % (2 * resolution));
      }
    }

    for(int i = 0; i < resolution * 2; i++) {
      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * i + 0] = resolution * 2 * (resolution - 1) + 0;
      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * i + 1] = i;
      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * i + 2] = (i + 1) % (2 * resolution);

      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * (resolution * 2 + i) + 0] =
	resolution * 2 * (resolution - 1) + 1;
      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * (resolution * 2 + i) + 1] =
	resolution * 2 * (resolution - 2) + i;
      indices[6 * (resolution * 2 * (resolution - 2)) + 3 * (resolution * 2 + i) + 2] =
	resolution * 2 * (resolution - 2) + ((i + 1) % (2 * resolution));
    }

    indexBuffer = wingine->createIndexBuffer(numDrawIndices * sizeof(uint32_t), indices.data());
  }

  void Model::initCube(std::initializer_list<WgAttribType> attribs_in, float size){
    const WgAttribType * attribs = std::begin(attribs_in);

    for(unsigned int i = 0; i < attribs_in.size(); i++){
      int attrib_size = WG_ATTRIB_TYPE_SIZE[attribs[i]];

      std::vector<float> data(attrib_size * 4 * 6); // Make each face with separate vertices

      if(attribs[i] == WG_ATTRIB_TYPE_POSITION){
	float s2 = size / 2;
        int dx = 0, dy = 1, dz = 2;
	for(int i = 0; i < 3; i++){
	  for(int j = 0; j < 4; j++){
	    data[attrib_size * (i * 2 * 4 + j) + dx] = (j  && j - 3) ? -s2 : s2;
	    data[attrib_size * (i * 2 * 4 + j) + dy] = (j & 2) ? -s2 : s2;
	    data[attrib_size * (i * 2 * 4 + j) + dz] = -s2;
	    data[attrib_size * (i * 2 * 4 + j) + 3] = 1.0f;

	    data[attrib_size * (i * 2 * 4 + j + 4) + dx] = (j && j - 3) ? -s2 : s2;
	    data[attrib_size * (i * 2 * 4 + j + 4) + dy] = (j & 2) ? s2 : -s2;
	    data[attrib_size * (i * 2 * 4 + j + 4) + dz] = s2;
	    data[attrib_size * (i * 2 * 4 + j + 4) + 3] = 1.0f;
	  }
	  std::swap(dx, dy);
	  std::swap(dy, dz);
	}
      } else if (attribs[i] == WG_ATTRIB_TYPE_NORMAL) {
        int dx = 0, dy = 1, dz = 2;
	for(int i = 0; i < 3; i++){
	  for(int j = 0; j < 4; j++){
	    data[attrib_size * (i * 2 * 4 + j) + dx] = 0.0f;
	    data[attrib_size * (i * 2 * 4 + j) + dy] = 0.0f;
	    data[attrib_size * (i * 2 * 4 + j) + dz] = -1.0f;
	    data[attrib_size * (i * 2 * 4 + j) + 3] = 0.0f;

	    data[attrib_size * (i * 2 * 4 + j + 4) + dx] = 0.0f;
	    data[attrib_size * (i * 2 * 4 + j + 4) + dy] = 0.0f;
	    data[attrib_size * (i * 2 * 4 + j + 4) + dz] = 1.0f;
	    data[attrib_size * (i * 2 * 4 + j + 4) + 3] = 0.0f;
	  }
	  std::swap(dx, dy);
	  std::swap(dy, dz);
	}
      } else if (attribs[i] == WG_ATTRIB_TYPE_TEXTURE) {
	for(int i = 0; i < 6; i++){
	  data[attrib_size * i * 4 + 0] = 0.0f;
	  data[attrib_size * i * 4 + 1] = 0.0f;

	  data[attrib_size * i * 4 + 2] = 1.0f;
	  data[attrib_size * i * 4 + 3] = 0.0f;
	  
	  data[attrib_size * i * 4 + 4] = 1.0f;
	  data[attrib_size * i * 4 + 5] = 1.0f;

	  data[attrib_size * i * 4 + 6] = 0.0f;
	  data[attrib_size * i * 4 + 7] = 1.0f;
	}
      }

      vertexAttribs.push_back(wingine->createVertexBuffer(attrib_size * 4 * 6 * sizeof(float), data.data()));
    }

    numDrawIndices = 3 * 2 * 6;
    indexOffset = 0;

    std::vector<uint32_t> indices(numDrawIndices);

    for(int i = 0; i < 6; i++){
      indices[3 * 2 * i + 0] = 4 * i + 0;
      indices[3 * 2 * i + 1] = 4 * i + 1;
      indices[3 * 2 * i + 2] = 4 * i + 2;

      indices[3 * 2 * i + 3] = 4 * i + 2;
      indices[3 * 2 * i + 4] = 4 * i + 3;
      indices[3 * 2 * i + 5] = 4 * i + 0;
    }

    indexBuffer = wingine->createIndexBuffer(numDrawIndices * sizeof(uint32_t), indices.data());
  }

  void Model::initQuad(std::initializer_list<WgAttribType> attribs_in,
		       const Vec3& side1, const Vec3& side2){
    const WgAttribType * attribs = std::begin(attribs_in);

    for(unsigned int i = 0; i < attribs_in.size(); i++){
      int attrib_size;
      switch(attribs[i]){
      case WG_ATTRIB_TYPE_POSITION:
	attrib_size = 4;
	break;
      case WG_ATTRIB_TYPE_NORMAL:
	attrib_size = 4;
	break;
      case WG_ATTRIB_TYPE_TEXTURE:
	attrib_size = 2;
	break;
      case WG_ATTRIB_TYPE_COLOR:
	attrib_size = 4;
	break;
      default:
	printf("Attrib type %d not implemented for quad\n", attribs[i]);
	exit(-1);
      }

      std::vector<float> data(attrib_size * 4 * 2); // Make each face with separate vertices

      if(attribs[i] == WG_ATTRIB_TYPE_POSITION){
	const Vec3 si1 = side1 / 2.f;
	const Vec3 si2 = side2 / 2.f;

        for(int i = 0; i < 4; i++){
	  const Vec3 v1 = (i & 1) ? - si1 : si1;
	  const Vec3 v2 = (i & 2) ? - si2 : si2;
	  for(int j = 0; j < 3; j++){
	    data[attrib_size * i + j] = v1(j) + v2(j);
	    data[attrib_size * 4 + attrib_size * i + j] = v1(j) + v2(j);
	  }
	  data[attrib_size * i + 3] = 1.0f;
	  data[attrib_size * 4 + attrib_size * i + 3] = 1.0f;
	}
      } else if (attribs[i] == WG_ATTRIB_TYPE_NORMAL) {
        const Vec3 norm = cross(side1, side2).normalized();
	for(int i = 0; i < 4; i++){
	  for(int j = 0; j < 3; j++){
	    data[attrib_size * i + j] = norm(j);
	    data[attrib_size * 4 + attrib_size * i + j] = -norm(j);
	  }
	  data[attrib_size * i + 3] = 0.0f;
	  data[attrib_size * 4 + attrib_size * i + 3] = 0.0f;
	}
      } else if (attribs[i] == WG_ATTRIB_TYPE_TEXTURE) {
        for(int i = 0; i < 2 * 4; i++){
	  data[attrib_size * i + 0] = (i & 1) ? 1.0f : 0.0f;
	  data[attrib_size * i + 1] = (i & 2) ? 1.0f : 0.0f;
	}
      } else if (attribs[i] == WG_ATTRIB_TYPE_COLOR) {
	for(int i = 0; i < 2 * 4; i++){
	  data[attrib_size * i + 0] = (i & 1) ? 1.0f : 0.0f;
	  data[attrib_size * i + 1] = 0.0f;
	  data[attrib_size * i + 2] = (i & 2) ? 1.0f : 0.0f;
	  data[attrib_size * i + 3] = 1.0f;
	}
      }

      vertexAttribs.push_back(wingine->createVertexBuffer(attrib_size * 4 * 2 * sizeof(float), data.data()));
    }

    numDrawIndices = 3 * 2 * 2;
    indexOffset = 0;
    std::vector<uint32_t> indices(numDrawIndices);

    indices[0] = 0;
    indices[1] = 2;
    indices[2] = 1;

    indices[3] = 1;
    indices[4] = 2;
    indices[5] = 3;

    indices[6] = 4;
    indices[7] = 5;
    indices[8] = 6;

    indices[9] = 5;
    indices[10] = 7;
    indices[11] = 6;

    indexBuffer = wingine->createIndexBuffer(numDrawIndices * sizeof(uint32_t), indices.data());
  }
}
