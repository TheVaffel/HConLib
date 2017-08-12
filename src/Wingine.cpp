#include <Wingine.h>
#include <external/vulkan/glsl_util.h>

#include <FlatAlg.h>


WingineCamera::WingineCamera(float horizontalFOVRadians, float invAspect, float near, float far){
  projection = flatalg::projection(horizontalFOVRadians, invAspect, near, far);
  view = Matrix4(FLATALG_MATRIX_IDENTITY);
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

Wingine::Wingine(const Winval& win){
  initVulkan(&win);
}

Wingine::~Wingine(){
  destroy_vulkan();
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
}


#define GET_INSTANCE_PROC_ADDR(instance, entry)			\
  (PFN_vk##entry)vkGetInstanceProcAddr(instance, "vk" #entry);

#define GET_DEVICE_PROC_ADDR(device, entry)		\
  (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry);

uint Wingine::get_memory_type_index( uint type_bits, VkFlags requirements_mask){
  for(uint i = 0; i < device_memory_props.memoryTypeCount; i++){
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
  instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

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

  VkResult res;

  res = vkCreateInstance(&inst_info, NULL, &instance);

  if( res == VK_ERROR_INCOMPATIBLE_DRIVER){
    printf("Cannot find a compatible Vulkan ICD\n");
    exit(-1);
  }
  
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
  
  for(uint i = 0; i < device_count; i++){
    vkGetPhysicalDeviceProperties(pDevs[i], &device_props);
    vkGetPhysicalDeviceMemoryProperties(pDevs[i], &device_memory_props);
    printf("Device %i: %s\n", i, device_props.deviceName);
    
    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &queue_family_count, NULL);
    queue_props = new VkQueueFamilyProperties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevs[i], &queue_family_count, queue_props);

    for(uint j = 0; j < queue_family_count; j++){
      if(queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
        physical_device = pDevs[i];
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

VkResult Wingine::init_device(){

  VkBool32 *pSupportsPresent = new VkBool32[queue_family_count];
  for(uint i = 0 ; i < queue_family_count; i++){
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &pSupportsPresent[i]);
  }

  graphics_queue_family_index = UINT32_MAX;
  present_queue_family_index = UINT32_MAX;
  
  for(uint i = 0; i < queue_family_count; i++){
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
    for(uint i = 0; i < queue_family_count; i++){
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
  float queue_priorities[1] = {0.0f};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;
  queue_info.queueFamilyIndex = graphics_queue_family_index;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = device_extension_names.size();
  device_info.ppEnabledExtensionNames = device_extension_names.data();
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = NULL;
  device_info.pEnabledFeatures = NULL;

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
  cmd_pool_info.queueFamilyIndex = graphics_queue_family_index;
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

  res = vkAllocateCommandBuffers(device, &cmd_alloc, &cmd_buffer);
  
  if(res != VK_SUCCESS){
    printf("Failed to allocate command buffer\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_surface(const Winval* win){
  VkXcbSurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.window = win->getWindow();
  surface_info.connection = win->getConnection();

  VkResult res = vkCreateXcbSurfaceKHR(instance, &surface_info, NULL, &surface);
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

  uint formatCount;
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

  if(formatCount == 1 &&  formats[0].format == VK_FORMAT_UNDEFINED){
    format = VK_FORMAT_B8G8R8A8_UNORM;
  } else if (formatCount > 1){
    format = formats[0].format;
  }else{
    printf("Did not get any format\n");
    exit(0);
  }

  delete[] formats;

  VkSurfaceCapabilitiesKHR caps;

  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
  if(res != VK_SUCCESS){
    printf("Could not get surface capabilities\n");
    exit(0);
  }

  uint presentModeCount;
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

  VkExtent2D swapchainExtent;

  if(caps.currentExtent.width == 0xFFFFFFFF){

    swapchainExtent.width = width;
    swapchainExtent.height = height;

    swapchainExtent.width = std::max(width, caps.minImageExtent.width);
    swapchainExtent.height = std::max(height, caps.minImageExtent.height);

    swapchainExtent.width = std::min(width, caps.maxImageExtent.width);
    swapchainExtent.height = std::min(height, caps.maxImageExtent.height);

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
  
  for(uint i = 0; i < sizeof(compositeAlphaFlags); i++){
    if(caps.supportedCompositeAlpha & compositeAlphaFlags[i]){
      compositeAlpha = compositeAlphaFlags[i];
      break;
    }
  }

  uint queueFamilyIndices[2] = {graphics_queue_family_index, present_queue_family_index};

  VkSwapchainCreateInfoKHR swapchain_ci = {};
  swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_ci.pNext = NULL;
  swapchain_ci.surface = surface;
  swapchain_ci.minImageCount = numSwaps;
  swapchain_ci.imageFormat = format;
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

  VkImage* swapchainImages = new VkImage[swapchain_image_count];

  res = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchainImages);

  if(res != VK_SUCCESS){
    printf("Failed to get swapchain images\n");
    exit(0);
  }

  swapchain_images = new VkImage[swapchain_image_count];
  for(uint i = 0; i < swapchain_image_count; i++){
    swapchain_images[i] = swapchainImages[i];
  }
  delete[] swapchainImages;

  swapchain_image_views = new VkImageView[swapchain_image_count];
  for(uint i = 0; i < swapchain_image_count; i++){
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

  VkMemoryRequirements mem_reqs;
  depth_buffer_format = depth_format;

  VkResult res = vkCreateImage(device, &image_info, NULL, &depth_buffer_image);
  if(res != VK_SUCCESS){
    printf("Could not create depth buffer\n");
    exit(0);
  }

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

void Wingine::init_uniform_buffer(){
  Matrix4 projection = flatalg::projection(45.f/180.0f*M_PI, 9.0f/16.0f, 0.1f, 100.0f);

  Matrix4 view = flatalg::lookAt(Vector3(-5, 3, -10),
				 Vector3(0, 0, 0),
				 Vector3(0, 1, 0));
  Matrix4 model = Matrix4(FLATALG_MATRIX_IDENTITY);
  Matrix4 clip = Matrix4(1.f, 0.f, 0.f, 0.f,
			 0.f, -1.f, 0.f, 0.f,
			 0.f, 0.f, 0.5f, 0.5f,
			 0.f, 0.f, 0.0f, 1.f);

  Matrix4 mvp = clip*projection*view*model;
  Matrix4 usableMvp = ~mvp;
  //printf("MVP = %s\n", mvp.str().c_str());
  
  VPCUniform = createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 4*4*sizeof(float));
  setBuffer(VPCUniform, &usableMvp, 4*4*sizeof(float));
  
  uniform_buffer = VPCUniform.buffer;
  uniform_memory = VPCUniform.memory;

  uniform_buffer_info.buffer = uniform_buffer;
  uniform_buffer_info.offset = 0;
  uniform_buffer_info.range = 4*4*sizeof(float);

  ModelTransformUniform = createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 4*4*sizeof(float));
  setBuffer(ModelTransformUniform, &model, 4*4*sizeof(float));
}

VkResult Wingine::init_descriptor_set_layouts(){
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

  desc_layout = new VkDescriptorSetLayout[NUM_DESCRIPTOR_SETS];
  VkResult res = vkCreateDescriptorSetLayout(device, &descriptor_layout, NULL, desc_layout);
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
  pPipelineLayoutCreateInfo.pSetLayouts = desc_layout;

  res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL, &pipeline_layout);
  if(res != VK_SUCCESS){
    printf("Could not create pipeline layout\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_descriptor_pool(){
  VkDescriptorPoolSize type_count[1];
  type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count[0].descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptor_pool_info = {};
  descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool_info.pNext = NULL;
  descriptor_pool_info.maxSets = 1;
  descriptor_pool_info.poolSizeCount = 1;
  descriptor_pool_info.pPoolSizes = type_count;

  VkResult res = vkCreateDescriptorPool(device, &descriptor_pool_info, NULL, &descriptor_pool);
  if(res != VK_SUCCESS){
    printf("Could not create descriptor pool\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_descriptor_set(){
  VkDescriptorSetAllocateInfo alloc_info[1];
  alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info[0].pNext = NULL;
  alloc_info[0].descriptorPool = descriptor_pool;
  alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
  alloc_info[0].pSetLayouts = desc_layout;

  descriptor_set = new VkDescriptorSet[NUM_DESCRIPTOR_SETS];
  VkResult res = vkAllocateDescriptorSets(device, alloc_info, descriptor_set);
  if(res != VK_SUCCESS){
    printf("Could not allocate descriptor sets\n");
    exit(0);
  }

  VkWriteDescriptorSet writes[1];
  writes[0] = {};
  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].pNext = NULL;
  writes[0].dstSet = descriptor_set[0];
  writes[0].descriptorCount = 1;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes[0].pBufferInfo = &uniform_buffer_info;
  writes[0].dstArrayElement = 0;
  writes[0].dstBinding = 0;
  vkUpdateDescriptorSets(device, 1, writes, 0, NULL);

  return res;
}

VkResult Wingine::init_render_pass(){
  VkAttachmentDescription attachments[2];
  attachments[0].format = format;
  attachments[0].samples = NUM_SAMPLES;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].flags = 0;

  attachments[1].format = depth_buffer_format;
  attachments[1].samples = NUM_SAMPLES;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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

  VkResult res = vkCreateRenderPass(device, &rp_info, NULL, &render_pass);
  if(res != VK_SUCCESS){
    printf("Could not create render pass\n");
    exit(0);
  }

  return res;
}

VkResult Wingine::init_shaders(){
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
    "  outColor = color;\n"
    "  //outColor = vec4(1.f, 1.f, 1.f, 0)*(1-gl_FragCoord.w*6) + vec4(0, 0, 0, 1);\n"
    "}\n";

  std::vector<unsigned int> vtx_spv;
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].pNext = NULL;
  shaderStages[0].pSpecializationInfo = NULL;
  shaderStages[0].flags = 0;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].pName = "main";

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
  VkResult res = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[0].module);
  if(res != VK_SUCCESS){
    printf("Could not create vertex shadermodule\n");
    exit(0);
  }

  std::vector<unsigned int> frag_spv;
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].pNext = NULL;
  shaderStages[1].pSpecializationInfo = NULL;
  shaderStages[1].flags = 0;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].pName = "main";

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
  res = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[1].module);
  if(res != VK_SUCCESS){
    printf("Could not create fragment shader module\n");
    exit(0);
  }

  glslang::FinalizeProcess();

  return res;
}

VkResult Wingine::init_framebuffers(){
  VkImageView attachments[2];
  attachments[1] = depth_buffer_view;
  
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = render_pass;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = attachments;
  fb_info.width = width;
  fb_info.height = height;
  fb_info.layers = 1;

  framebuffers = new VkFramebuffer[swapchain_image_count];

  VkResult res;
  for(uint i = 0 ; i < swapchain_image_count; i++){
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
  vkDestroyInstance(instance, NULL);
}

void Wingine::destroy_device(){
  vkDestroyDevice(device, NULL);
}

void Wingine::destroy_command_buffers(){
  vkFreeCommandBuffers(device, cmd_pool, 1, &cmd_buffer);
  vkDestroyCommandPool(device, cmd_pool, NULL);
}

void Wingine::destroy_swapchain(){
  for(uint i = 0; i < swapchain_image_count;i++){
    vkDestroyImageView(device, swapchain_image_views[i], NULL);
  }
  
  delete[] swapchain_image_views;
  delete[] swapchain_images;
  vkDestroySwapchainKHR(device, swapchain, NULL);
  
  vkDestroySurfaceKHR(instance, surface, NULL);
}

void Wingine::destroy_depth_buffer(){
  vkDestroyImageView(device, depth_buffer_view, NULL);
  vkDestroyImage(device, depth_buffer_image, NULL);
  vkFreeMemory(device, depth_buffer_memory, NULL);
}

void Wingine::destroy_uniform_buffer(){
  vkDestroyBuffer(device, uniform_buffer, NULL);
  vkFreeMemory(device, uniform_memory, NULL);
}

void Wingine::destroy_descriptor_set_layouts(){
  for(int i = 0; i < NUM_DESCRIPTOR_SETS; i++) vkDestroyDescriptorSetLayout(device, desc_layout[i], NULL);
  vkDestroyPipelineLayout(device, pipeline_layout, NULL);
}

void Wingine::destroy_descriptor_set(){
  vkDestroyDescriptorPool(device, descriptor_pool, NULL); 
}

void Wingine::destroy_render_pass(){
  vkDestroyRenderPass(device, render_pass, NULL);
}

void Wingine::destroy_shaders(){
  vkDestroyShaderModule(device, shaderStages[0].module, NULL);
  vkDestroyShaderModule(device, shaderStages[1].module, NULL);
}

void Wingine::destroy_framebuffers(){
  for(uint i = 0; i < swapchain_image_count; i++){
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

/*void Wingine::updateMVP(const Matrix4& nmvp){
  unsigned char *pData;
  
  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(device, uniform_buffer, &mem_reqs);
  
  VkResult res = vkMapMemory(device, uniform_memory, 0, mem_reqs.size, 0, (void**)&pData);
  if(res != VK_SUCCESS){
  printf("Could not map memory\n");
  exit(0);
  }

  memcpy(pData, &nmvp, sizeof(nmvp));

  vkUnmapMemory(device, uniform_memory);
  }*/

void Wingine::setCamera(WingineCamera& camera){
  Matrix4 VPC = camera.getRenderMatrix();
  setBuffer(VPCUniform, (void*)&VPC, 4*4*sizeof(float));
}


WingineBuffer Wingine::createBuffer( uint usage, uint size){
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

VkResult Wingine::setBuffer( const WingineBuffer& buffer, const void* data, uint size){
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

WingineUniform Wingine::createUniform(uint size){
  WingineUniform uniform;
  uniform.buffer = createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size);
  uniform.bufferInfo.buffer = uniform.buffer.buffer;
  uniform.bufferInfo.offset = 0;
  uniform.bufferInfo.range = size;

  return uniform;
}

void Wingine::setUniform(const WingineUniform& uniform, void* data, uint size){
  setBuffer(uniform.buffer, data, size);
}

void Wingine::pipeline_setup_color_renderer( WinginePipelineSetup* setup){
  
  memset(setup->dynamicStateEnables, 0, sizeof(setup->dynamicStateEnables));
  setup->dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  setup->dynamicState.pNext = NULL;
  setup->dynamicState.flags = 0;
  setup->dynamicState.pDynamicStates = setup->dynamicStateEnables;
  setup->dynamicState.dynamicStateCount = 0;
  
  setup->vi_bindings[0].binding = 0;
  setup->vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  setup->vi_bindings[0].stride = 4*sizeof(float);

  setup->vi_bindings[1].binding = 1;
  setup->vi_bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  setup->vi_bindings[1].stride = 4*sizeof(float);
  
  setup->vi_attribs[0].binding = 0;
  setup->vi_attribs[0].location = 0;
  setup->vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  setup->vi_attribs[0].offset = 0;
  
  setup->vi_attribs[1].binding = 1;
  setup->vi_attribs[1].location = 1;
  setup->vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  setup->vi_attribs[1].offset = 0;

  setup->vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  setup->vi.pNext = NULL;
  setup->vi.flags = 0;
  setup->vi.vertexBindingDescriptionCount = 2;
  setup->vi.pVertexBindingDescriptions = setup->vi_bindings;
  setup->vi.vertexAttributeDescriptionCount = 2;
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

  setup->createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  setup->createInfo.pNext = NULL;
  setup->createInfo.layout = pipeline_layout;
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
  setup->createInfo.pStages = shaderStages;
  setup->createInfo.stageCount = 2;
  setup->createInfo.renderPass = render_pass;
  setup->createInfo.subpass = 0;
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
  find_device();
  init_surface( win);
  init_device();
  init_device_queue();
  init_swapchain();
  init_command_buffers();
  init_depth_buffer();
  init_uniform_buffer();
  init_descriptor_set_layouts();
  
  init_render_pass();
  init_shaders();
  init_framebuffers();
  init_descriptor_pool();
  init_descriptor_set();
  init_pipeline_cache();
  
  create_pipeline_color_renderer( &color_pipeline);
}

void Wingine::destroy_vulkan(){
  destroy_pipeline();
  destroy_pipeline_cache();
  destroy_framebuffers();
  destroy_shaders();
  destroy_render_pass();
  destroy_descriptor_set();
  destroy_descriptor_set_layouts();
  destroy_uniform_buffer();
  destroy_depth_buffer();
  destroy_swapchain();
  destroy_command_buffers();
  destroy_device();
  destroy_instance();
}

void Wingine::render_generic( VkPipeline pipeline, const WingineBuffer& vertexBuffer, const WingineBuffer& colorBuffer, const WingineBuffer& indexBuffer, const Matrix4& model){

  setBuffer(ModelTransformUniform, &model, 4*4*sizeof(float));

  VkCommandBufferBeginInfo cmd_buf_info = {};
  cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_buf_info.pNext = NULL;
  cmd_buf_info.flags = 0;
  cmd_buf_info.pInheritanceInfo = NULL;

  VkClearValue clear_values[2];
  clear_values[0].color.float32[0] = 0.2f;
  clear_values[0].color.float32[1] = 0.2f;
  clear_values[0].color.float32[2] = 0.2f;
  clear_values[0].color.float32[3] = 0.2f;
  clear_values[1].depthStencil.depth = 1.0;
  clear_values[1].depthStencil.stencil = 0;
  
  VkClearAttachment clearAttachments[2];
  clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  clearAttachments[0].colorAttachment = 0;
  clearAttachments[0].clearValue = clear_values[0];

  clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  clearAttachments[1].clearValue = clear_values[1];

  VkRect2D screenRect;
  screenRect.extent.width = width;
  screenRect.extent.height = height;
  screenRect.offset.x = 0;
  screenRect.offset.y = 0;

  VkClearRect clearRect;
  clearRect.rect = screenRect;
  clearRect.baseArrayLayer = 0;
  clearRect.layerCount = 1;

  vkResetCommandBuffer(cmd_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  VkResult res = vkBeginCommandBuffer(cmd_buffer, &cmd_buf_info);
  if(res != VK_SUCCESS){
    printf("Could not begin command buffer\n");
    exit(0);
  }

  const VkDeviceSize offsets[2] = {0, 0};
  
  VkSemaphore imageAcquiredSemaphore;
  VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  imageAcquiredSemaphoreCreateInfo.pNext = NULL;
  imageAcquiredSemaphoreCreateInfo.flags = 0;

  res = vkCreateSemaphore(device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
  if(res != VK_SUCCESS){
    printf("Could not create semaphore\n");
    exit(0);
  }

  res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
			      &current_buffer);

  if(res != VK_SUCCESS){
    printf("Could not get next image from swapchain\n");
    exit(0);
  }

  VkRenderPassBeginInfo rp_begin = {};
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = NULL;
  rp_begin.renderPass = render_pass;
  rp_begin.framebuffer = framebuffers[current_buffer];
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = width;
  rp_begin.renderArea.extent.height = height;
  rp_begin.clearValueCount = 2;
  rp_begin.pClearValues = clear_values;

  VkBuffer vertexBuffers[] = {vertexBuffer.buffer, colorBuffer.buffer};

  vkCmdBeginRenderPass(cmd_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
			  descriptor_set, 0, NULL);
  vkCmdBindVertexBuffers(cmd_buffer, 0,
			 2,
			 vertexBuffers,
			 offsets);
  vkCmdBindIndexBuffer(cmd_buffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdClearAttachments(cmd_buffer,
  			2,
  			clearAttachments,
  			1,
  			&clearRect);
			

  viewport.height = (float)height;
  viewport.width = (float)width;
  viewport.minDepth = (float)0.0f;
  viewport.maxDepth = (float)1.0f;
  viewport.x = 0;
  viewport.y = 0;
  vkCmdSetViewport(cmd_buffer, 0, NUM_VIEWPORTS, &viewport);

  scissor.extent.width = width;
  scissor.extent.height = height;
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetScissor(cmd_buffer, 0, NUM_SCISSORS, &scissor);

  //vkCmdDraw(cmd_buffer, 18, 1, 0, 0);
  vkCmdDrawIndexed(cmd_buffer, 2*3, 1, 0, 0, 0);
  vkCmdEndRenderPass(cmd_buffer);
  res = vkEndCommandBuffer(cmd_buffer);

  /* Queue the command buffer for execution */
  const VkCommandBuffer cmd_bufs[] = {cmd_buffer};
  VkFenceCreateInfo fenceInfo;
  VkFence drawFence;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = 0;
  vkCreateFence(device, &fenceInfo, NULL, &drawFence);

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
    
  res = vkQueueSubmit(graphics_queue, 1, submit_info, drawFence);
  if(res != VK_SUCCESS){
    printf("Could not submit to graphics queue\n");
    exit(0);
  }

  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = NULL;
  present.swapchainCount = 1;
  present.pSwapchains = &swapchain;
  present.pImageIndices = &current_buffer;
  present.pWaitSemaphores = NULL;
  present.waitSemaphoreCount = 0;
  present.pResults = NULL;

  do {
    res = vkWaitForFences(device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  if(res != VK_SUCCESS){
    printf("Drawing was not a success\n");
    exit(0);
  }

  res = vkQueuePresentKHR(present_queue, &present);

  if(res != VK_SUCCESS){
    printf("Could not queue present\n");
    exit(0);
  }

  vkDestroyFence(device, drawFence, NULL);

  vkDestroySemaphore(device, imageAcquiredSemaphore, NULL);
}

void Wingine::renderColor(const WingineBuffer& vertexBuffer, const WingineBuffer& colorBuffer, const WingineBuffer& indexBuffer, const Matrix4& model){
  render_generic(color_pipeline, vertexBuffer, colorBuffer, indexBuffer, model);
}

