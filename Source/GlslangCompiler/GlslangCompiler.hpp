#pragma once

#include <glslang/Include/glslang_c_interface.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace ShaderCompiler::Vulkan
{
    struct ShaderCompilerDesc {
        const char *entryFile = nullptr;
        std::vector<std::string> includeDirs;
        std::vector<std::string> defines;
    };

    void initializeGlslang();
    VkShaderStageFlagBits glslangShaderStageToVulkan(glslang_stage_t sh);
    glslang_stage_t glslangShaderStageFromFileName(const char* fileName);
    size_t compileShaderFile(const ShaderCompilerDesc &desc, std::vector<unsigned int> &SPIRV);
}
