#pragma once

#include <string>
#include <vector>

namespace ShaderCompiler::ShaderUtils
{
	int endsWith(const char* s, const char* part);

	std::string readShaderFile(const char* fileName, const std::vector<std::string> &includeDirs = {});

	void printShaderSource(const char* text);
}
