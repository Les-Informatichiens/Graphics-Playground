cmake_minimum_required(VERSION 3.26)
project(renderer VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

add_library(
        ${PROJECT_NAME} STATIC
        src/renderer/openGLRenderer.cpp
        include/renderer/iRenderer.h
        include/renderer/renderAPI/openGLRenderer.h
        include/renderer/renderAPI/shader.h
        include/renderer/renderAPI/vertexBuffer.h
        include/renderer/renderAPI/indexBuffer.h
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Dependencies ========================================================================================
include(cmake/CPM.cmake)

# glew ================================================================================================
if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PUBLIC "-sUSE_WEBGL2=1")
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

# end dependencies ====================================================================================

# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================
