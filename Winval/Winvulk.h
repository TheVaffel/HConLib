#ifndef INCLUDED_WINVULK
#define INCLUDED_WINVULK

#include <unistd.h>
#include "vulkan/vulkan.h"

#include <iostream> // cout
#include <cstdlib> //exit


struct vulkan_state {
  VkInstance instance;
};

#define GET_INSTANCE_PROC_ADDR(instance, entry)			\
  (PFN_vk##entry)vkGetInstanceProcAddr(instance, "vk" #entry);

#define GET_DEVICE_PROC_ADDR(device, entry)		\
  (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry);

VkResult init_global_extension_properties();
VkResult init_global_layer_properties();

int init_vulkan(vulkan_state*, Winval*);

VkResult winvulk_init_instance(vulkan_state*, Winval*);
VkResult winvulk_init_device(vulkan_state*, Winval*)

void winvulk_destroy_instance(vulkan_state*);

#endif //INCLUDED_WINVULK

#ifdef WINVAL_VULKAN_IMPLEMENTATION

VkResult winvulk_init_instance(vulkan_state* vs, Winval* win){
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
  inst_info.ppEnabledExtensionNames = NULL;
  inst_info.enabledLayerCount = 0;
  inst_info.ppEnabledLayerNames = NULL;

  VkResult res;

  res = vkCreateInstance(&inst_info, NULL, &(vs->instance));

  if( res == VK_ERROR_INCOMPATIBLE_DRIVER){
    std::cout<<"Cannot find a compatible Vulkan ICD"<<std::endl;
    exit(-1);
  }
}

winvulk_init_device(vs, win){
  unsigned int device_count;
  
  VkResult res = vkEnumeratePhysicalDevices(vs->instance, &device_count, NULL);
  VkPhysicalDevice* pDevs = new VkPhysicalDevice[device_count];
  res = vkEnumeratePhysicalDevices(vs->instance, &device_count, pDevs);
  std::cout<<"Number of devices: "<<device_count<<std::endl;

  for(int i = 0; i < device_count; i++){
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(pDevs[i], &props);
    std::cout<<props.deviceName<<std::endl;
  }
}

void winvulk_destroy_instance(vulkan_state* vs){
  vkDestroyInstance(vs->instance, NULL);
}


int winvulk_init_vulkan(vulkan_state* vs, Winval* win){
  winvulk_init_instance(vs, win);
  winvulk_init_device(vs, win);
  
  
}

int winvulk_destroy_vulkan(vulkan_state* vs){
  winvulk_destroy_instance(vs);
}


#endif //WINVAL_VULKAN_IMPLEMENTATION
