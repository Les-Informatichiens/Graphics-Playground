cmake_minimum_required(VERSION 3.26)
project(engine VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
        ${PROJECT_NAME} STATIC
        src/engine/TestRenderPass.cpp
        include/engine/TestRenderPass.h
        include/engine/graphics/Renderer.h
        src/engine/graphics/Renderer.cpp
        include/engine/EngineInstance.h
        src/engine/EngineInstance.cpp
        src/engine/MeshRenderer.cpp
        include/engine/MeshRenderer.h
        src/engine/graphics/Renderable.cpp
        include/engine/graphics/Renderable.h
        src/engine/Mesh.cpp
        include/engine/Mesh.h
        src/engine/Transform.cpp
        include/engine/Transform.h
        src/engine/Model.cpp
        include/engine/Model.h
        src/engine/Camera.cpp
        include/engine/Camera.h
        include/engine/stb_image.h
        src/engine/SceneNode.cpp
        include/engine/SceneNode.h
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# dependencies =========================================================================================
include(cmake/CPM.cmake)

# glm =================================================================================================
CPMAddPackage(
        NAME "glm"
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "0.9.9.8"
)

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
# =====================================================================================================

## assimp ===============================================================================================
#CPMAddPackage(
#        NAME "assimp"
#        GITHUB_REPOSITORY "assimp/assimp"
#        GIT_TAG "v5.3.1"
#        OPTIONS
#        "ASSIMP_BUILD_TESTS OFF"
#        "ASSIMP_BUILD_SAMPLES OFF"
#        "ASSIMP_BUILD_NO_EXPORT ON"
#        "ASSIMP_WARNINGS_AS_ERRORS OFF"
#        "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
#        "ASSIMP_BUILD_OBJ_IMPORTER ON",
#        "ASSIMP_BUILD_M3D_EXPORTER OFF"
#        "ASSIMP_BUILD_IFC_IMPORTER OFF"
#        "ASSIMP_BUILD_ASSXML_EXPORTER OFF"
#        "ASSIMP_BUILD_GLTF_EXPORTER OFF"
#        "ASSIMP_BUILD_OBJ_EXPORTER OFF"
#        "ASSIMP_BUILD_OPENGEX_EXPORTER OFF"
#        "ASSIMP_BUILD_PLY_EXPORTER OFF"
#        "ASSIMP_BUILD_STL_EXPORTER OFF"
#        "ASSIMP_BUILD_FBX_EXPORTER OFF"
#        "ASSIMP_BUILD_X3D_EXPORTER OFF"
#        "ASSIMP_BUILD_XGL_EXPORTER OFF"
#        "ASSIMP_BUILD_X_EXPORTER OFF"
#        "ASSIMP_BUILD_STEP_EXPORTER OFF"
#        "ASSIMP_BUILD_ASSJSON_EXPORTER OFF"
#        "ASSIMP_BUILD_3MF_EXPORTER OFF"
#        "ASSIMP_BUILD_3DS_EXPORTER OFF"
#        "ASSIMP_BUILD_COLLADA_EXPORTER OFF"
#        "ASSIMP_BUILD_PBRT_EXPORTER OFF"
#        "ASSIMP_BUILD_ASSBIN_EXPORTER OFF"
#
#
#)
#target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
## =====================================================================================================

# end dependencies =====================================================================================

# renderer =============================================================================================
target_link_libraries(${PROJECT_NAME} PUBLIC graphicsAPI)
# ======================================================================================================

# End dependencies =====================================================================================