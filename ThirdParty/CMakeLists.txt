set(ThirdParty_Libs)
set(ThirdParty_IncludeDirs)

set(MessageQuiet OFF)
function(message)
    if (NOT MessageQuiet)
        _message(${ARGN})
    endif ()
endfunction()

macro(quiet Status)
    set(MessageQuiet ${Status})
endmacro()

macro(quiet_add_subdirectory Subdir)
    quiet(ON)
    add_subdirectory(${Subdir})
    quiet(OFF)
endmacro()

###############
#   glslang   #
###############
if (USE_GLSLANG)
    set(Glslang_Root ${CMAKE_CURRENT_SOURCE_DIR}/glslang)

	#set(ENABLE_GLSLANG_BINARIES OFF "Do not build glslang binaries")
    #set(ENABLE_SPVREMAPPER OFF "Do not build SPIRV remapper")
	#set(ENABLE_SPIRV OFF "Do not build SPIRV")
	#set(ENABLE_HLSL OFF "Do not build HLSL")
    set(ENABLE_CTEST OFF)
    set(ENABLE_GLSLANG_JS OFF)
	set(ENABLE_OPT OFF)
    
    #find_package(glslang CONFIG REQUIRED GLOBAL)

    quiet_add_subdirectory(${Glslang_Root})
	#add_subdirectory(glslang)
	
	#set(ThirdParty_IncludeDirs ${Glslang_Root})

    #group_targets(TARGETS_PATH ${Glslang_Root})

    list(APPEND ThirdParty_Libs 
		glslang
        SPIRV
        glslang-default-resource-limits)
endif ()

set(ThirdParty_Libs ${ThirdParty_Libs} PARENT_SCOPE)
set(ThirdParty_IncludeDirs ${ThirdParty_IncludeDirs} PARENT_SCOPE)
