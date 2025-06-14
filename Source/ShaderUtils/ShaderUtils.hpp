#pragma once

#include <string>

namespace ShaderCompiler::ShaderUtils
{
	int endsWith(const char* s, const char* part);

	std::string readShaderFile(const char* fileName, const std::string& shaderDirectoryName = std::string());

	void printShaderSource(const char* text);
}
