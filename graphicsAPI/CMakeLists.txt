cmake_minimum_required(VERSION 3.26)
project(graphicsAPI VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
        ${PROJECT_NAME} STATIC
        include/graphicsAPI/common/Buffer.h
        include/graphicsAPI/opengl/Buffer.h
        src/opengl/Buffer.cpp
        src/common/ShaderModule.cpp
        include/graphicsAPI/common/ShaderModule.h
        include/graphicsAPI/common/Device.h
        include/graphicsAPI/common/CommandBuffer.h
        include/graphicsAPI/common/CommandPool.h
        include/graphicsAPI/common/GraphicsPipeline.h
        include/graphicsAPI/common/RenderPass.h
        include/graphicsAPI/common/VertexInputState.h
        include/graphicsAPI/common/Util.h
        include/graphicsAPI/common/Texture.h
        include/graphicsAPI/common/Framebuffer.h
        include/graphicsAPI/common/PlatformDevice.h
        src/opengl/Device.cpp
        include/graphicsAPI/opengl/Device.h
        src/opengl/PlatformDevice.cpp
        include/graphicsAPI/opengl/PlatformDevice.h
        src/opengl/Context.cpp
        include/graphicsAPI/opengl/Context.h
        include/graphicsAPI/common/DeviceFeatures.h
        src/opengl/GraphicsPipeline.cpp
        src/opengl/GraphicsPipeline.h
        src/opengl/ShaderStage.cpp
        src/opengl/ShaderStage.h
        src/common/ShaderStage.cpp
        src/opengl/ShaderModule.cpp
        src/opengl/ShaderModule.h
        src/common/VertexInputState.cpp
        src/opengl/VertexInputState.cpp
        src/opengl/VertexInputState.h
        src/opengl/GraphicsPipelineReflection.cpp
        src/opengl/GraphicsPipelineReflection.h
        src/util/hashed_string.h
        src/opengl/ShaderModuleReflection.cpp
        src/opengl/ShaderModuleReflection.h
        include/graphicsAPI/common/Common.h
        src/opengl/VertexArrayObject.cpp
        src/opengl/VertexArrayObject.h
        src/opengl/CommandBuffer.cpp
        src/opengl/CommandBuffer.h
        src/opengl/CommandPool.cpp
        src/opengl/CommandPool.h
        src/opengl/UniformBinder.cpp
        src/opengl/UniformBinder.h
        src/opengl/Framebuffer.cpp
        src/opengl/Framebuffer.h
        src/opengl/Texture.cpp
        src/opengl/Texture.h
        src/opengl/Renderbuffer.cpp
        src/opengl/Renderbuffer.h
        src/opengl/TextureBuffer.cpp
        src/opengl/TextureBuffer.h
        include/graphicsAPI/common/DepthStencilState.h
        src/opengl/DepthStencilState.cpp
        src/opengl/DepthStencilState.h
        include/graphicsAPI/common/TextureStructures.h
        src/common/TextureStructures.cpp
        src/common/Texture.cpp
        include/graphicsAPI/common/SamplerState.h
        src/common/SamplerState.cpp
        src/opengl/SamplerState.cpp
        src/opengl/SamplerState.h
        include/graphicsAPI/common/Uniform.h
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Dependencies ========================================================================================
include(cmake/CPM.cmake)

# glew ================================================================================================
if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGL2=1")
else ()
    CPMAddPackage(
            NAME "GLEW"
            GITHUB_REPOSITORY "Perlmint/glew-cmake"
            GIT_TAG "glew-cmake-2.2.0"
            OPTIONS
            "glew-cmake_BUILD_SHARED OFF"
            "glew-cmake_BUILD_STATIC ON"
            "glew-cmake_ONLY_LIBS ON"
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC libglew_static)
endif ()
# =====================================================================================================

## spirv-cross =========================================================================================
#CPMAddPackage("gh:KhronosGroup/SPIRV-Cross#main")
#target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-core spirv-cross-glsl)
## =====================================================================================================

# SPIRV-Headers =======================================================================================
#CPMAddPackage(
#        NAME "SPIRV-Headers"
#        GITHUB_REPOSITORY "KhronosGroup/SPIRV-Headers"
#        GIT_TAG "main"
#        OPTIONS
#        "BUILD_TESTS OFF"
#)
# =====================================================================================================

# SPIRV-Tools =========================================================================================
#CPMAddPackage(
#        NAME "SPIRV-Tools"
#        GITHUB_REPOSITORY "KhronosGroup/SPIRV-Tools"
#        GIT_TAG "main"
#        OPTIONS
#        "SPIRV_SKIP_EXECUTABLES ON"
#        "SPIRV_SKIP_TESTS ON"
#        "BUILD_SHARED_LIBS ON"
#        "SPIRV_TOOLS_BUILD_STATIC OFF"
##        "SPIRV_SKIP_INSTALL ON"
#)
#target_link_libraries(${PROJECT_NAME} PRIVATE SPIRV-Tools)
# =====================================================================================================

# glslang =============================================================================================
#CPMAddPackage(
#        NAME "glslang"
#        GITHUB_REPOSITORY "KhronosGroup/glslang"
#        GIT_TAG "14.0.0"
#        OPTIONS
#        "GLSLANG_TESTS OFF"
#        "ENABLE_GLSLANG_BINARIES OFF"
#        "BUILD_SHARED_LIBS ON"
##        "ENABLE_HLSL OFF"
##        "ENABLE_OPT OFF"
##        "ENABLE_SPVREMAPPER OFF"
##        "ENABLE_AMD_EXTENSIONS OFF"
##        "ENABLE_NV_EXTENSIONS OFF"
##        "ENABLE_GLSLANG_JS OFF"
##        "ENABLE_GLSLANG_WEB OFF"
#)
# =====================================================================================================


# shaderc =============================================================================================
#CPMAddPackage(
#        NAME "shaderc"
#        GITHUB_REPOSITORY "google/shaderc"
#        GIT_TAG "v2023.8"
#        OPTIONS
#        "SHADERC_SKIP_TESTS ON"
#        "SHADERC_SKIP_INSTALL ON"
#        "SHADERC_SKIP_COPYRIGHT_CHECK ON"
#        "SHADERC_SKIP_EXAMPLES ON"
#
#)
#target_link_libraries(${PROJECT_NAME} PRIVATE shaderc)
# =====================================================================================================

# fmt =================================================================================================
CPMAddPackage(
        NAME "fmt"
        GITHUB_REPOSITORY "fmtlib/fmt"
        GIT_TAG "10.2.1"
)
#target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
# =====================================================================================================

# cpptrace ============================================================================================
CPMAddPackage(
        NAME "cpptrace"
        GITHUB_REPOSITORY "jeremy-rifkin/cpptrace"
        VERSION "0.3.1"
)
# =====================================================================================================

# libassert ===========================================================================================
CPMAddPackage(
        NAME "libassert"
        GITHUB_REPOSITORY "jeremy-rifkin/libassert"
        VERSION "1.2.2"
        OPTIONS
        "ASSERT_STATIC"
        "ASSERT_USE_EXTERNAL_CPPTRACE ON"
        "CMAKE_SKIP_INSTALL_RULES ON"
)
#target_link_libraries(${PROJECT_NAME} PRIVATE assert)
# =====================================================================================================

# End dependencies =====================================================================================


# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================
