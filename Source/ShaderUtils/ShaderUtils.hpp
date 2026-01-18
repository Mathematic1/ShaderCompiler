#pragma once

#include <string>
#include <span>

namespace ShaderCompiler::ShaderUtils
{
	int endsWith(const char* s, const char* part);

	std::string readShaderFile(std::string_view fileName, std::span<const std::string> includeDirs = {})
            noexcept(false);

	void printShaderSource(const char* text);
}
