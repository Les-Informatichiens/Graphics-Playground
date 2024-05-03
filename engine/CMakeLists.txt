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
        src/engine/Stage.cpp
        include/engine/Stage.h
        src/engine/Scene.cpp
        include/engine/Scene.h
        src/engine/SceneRenderer.cpp
        include/engine/SceneRenderer.h
        include/engine/SceneRenderData.h
        src/engine/EntityView.cpp
        include/engine/EntityView.h
        include/engine/util/UUID.h
        src/engine/util/UUID.cpp
        src/engine/components/MeshComponent.cpp
        include/engine/components/MeshComponent.h
        src/engine/components/CameraComponent.cpp
        include/engine/components/CameraComponent.h
        src/engine/graphics/RenderTarget.cpp
        include/engine/graphics/RenderTarget.h
        src/engine/graphics/Material.cpp
        include/engine/graphics/Material.h
        src/engine/graphics/ShaderProgram.cpp
        include/engine/graphics/ShaderProgram.h
        src/engine/graphics/VertexData.cpp
        include/engine/graphics/VertexData.h
        src/engine/graphics/VertexDataLayout.cpp
        include/engine/graphics/VertexDataLayout.h
        src/engine/LineRenderer.cpp
        include/engine/LineRenderer.h
        src/engine/util/Math.cpp
        include/engine/util/Math.h
        src/engine/Bounds.cpp
        include/engine/Bounds.h
        src/engine/Input.cpp
        include/engine/Input.h
        src/engine/ResourceManager.cpp
        include/engine/ResourceManager.h
        src/engine/graphics/TextureResource.cpp
        include/engine/graphics/TextureResource.h
        src/engine/Resource.cpp
        include/engine/Resource.h
        src/engine/graphics/MeshResource.cpp
        include/engine/graphics/MeshResource.h
        src/engine/ResourceHandle.cpp
        include/engine/ResourceHandle.h
        src/engine/graphics/MaterialResource.cpp
        include/engine/graphics/MaterialResource.h
        src/engine/graphics/DeviceManager.cpp
        include/engine/graphics/DeviceManager.h
        src/engine/ImageResource.cpp
        include/engine/ImageResource.h
        src/engine/components/LightComponent.cpp
        include/engine/components/LightComponent.h
        src/engine/Light.cpp
        include/engine/Light.h
        src/engine/graphics/ShaderResource.cpp
        include/engine/graphics/ShaderResource.h
        src/engine/TempResourceInitializer.h
        src/engine/util/Ray.cpp
        include/engine/util/Ray.h
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

# nlohmann_json ========================================================================================
CPMAddPackage(
        NAME "nlohmann_json"
        GITHUB_REPOSITORY "nlohmann/json"
        GIT_TAG "v3.11.3"
)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
# =====================================================================================================

# EnTT ================================================================================================
CPMAddPackage(
        NAME EnTT
        VERSION 3.13.1
        GITHUB_REPOSITORY skypjack/entt
        # EnTT's CMakeLists screws with configuration options
        DOWNLOAD_ONLY True
)

if (EnTT_ADDED)
    add_library(EnTT INTERFACE)
    target_include_directories(EnTT INTERFACE ${EnTT_SOURCE_DIR}/src)
    target_link_libraries(${PROJECT_NAME} PUBLIC EnTT)
endif()
# =====================================================================================================


## lua ================================================================================================
CPMAddPackage(
        NAME "sol2"
        GITHUB_REPOSITORY "ThePhD/sol2"
        VERSION "3.3.0"
)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2::sol2)
## ====================================================================================================

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