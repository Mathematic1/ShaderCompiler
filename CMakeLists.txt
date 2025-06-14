cmake_minimum_required(VERSION 3.24)

project(ShaderCompiler VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SHADER_COMPILER ON)

#############################################
#   Option to control third party modules   #
#############################################
option(USE_GLSLANG "Glslang" ON)
option(LOG_ALL "Log all results" OFF)

list(APPEND ShaderCompiler_Source_File_Ext
        /*.cpp
        /*.hpp
		/*.h
        /*.ini)

if(${SHADER_COMPILER})
	set(ShaderCompiler_Sources_Ext ${ShaderCompiler_Source_File_Ext})
	list(TRANSFORM ShaderCompiler_Sources_Ext PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Source)
	file(GLOB_RECURSE ShaderCompiler_Common_Sources ${ShaderCompiler_Sources_Ext})
    list(APPEND ShaderCompiler_Sources ${ShaderCompiler_Common_Sources})
    list(APPEND ShaderCompiler_Include_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Source)
endif ()

###################
#   Third party   #
###################
add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty)

find_package(Vulkan REQUIRED)
list(APPEND ThirdParty_Libs
        Vulkan::Vulkan)

# add_library(ShaderCompiler STATIC)
add_library(ShaderCompiler)
target_sources(ShaderCompiler
        PRIVATE ${ShaderCompiler_Sources})
target_include_directories(ShaderCompiler
		PUBLIC 
		${ShaderCompiler_Include_Dir}
		${ThirdParty_IncludeDirs})

target_link_libraries(ShaderCompiler
        PRIVATE		
        ${ThirdParty_Libs}
		#glslang::glslang
        #glslang::glslang-default-resource-limits
        #glslang::SPIRV
        #glslang::SPVRemapper
		)

target_compile_features(ShaderCompiler
        PRIVATE cxx_std_20)
