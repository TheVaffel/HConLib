#include "vulkan.h"
#include "SPIRV/GlslangToSpv.h"

//// Shamelessly copied from LunarG's Vulkan tutorial

void init_resources(TBuiltInResource &Resources);

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);

//
// Compile a given string containing GLSL into SPV for use by VK
// Return value of false means an error was encountered.
//
bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv);
