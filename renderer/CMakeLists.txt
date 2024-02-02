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

# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================
