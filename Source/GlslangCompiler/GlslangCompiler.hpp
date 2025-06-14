#pragma once

#include <glslang/Include/glslang_c_interface.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace ShaderCompiler::Vulkan
{
    VkShaderStageFlagBits glslangShaderStageToVulkan(glslang_stage_t sh);
    glslang_stage_t glslangShaderStageFromFileName(const char* fileName);
    size_t compileShaderFile(const char* file, std::vector<unsigned int>& SPIRV);
}
