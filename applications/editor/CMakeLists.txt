cmake_minimum_required(VERSION 3.26)
project(editor VERSION 0.1)

include(cmake/Utils.cmake)
init_output_directories()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
        ${PROJECT_NAME}
        src/main.cpp
        src/application.cpp
        src/imgui/ImGuiRenderer.cpp
        src/imgui/ImGuiInstance.cpp
        src/imgui/ImGuiContextAdapter.cpp
        src/imgui/scenePanel/SceneEditor.cpp
        src/imgui/raytracerPanel/rt.cpp

)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")  # Added /bigobj to handle large number of sections in object files
endif()

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-s WASM=1 -sUSE_GLFW=3 -sUSE_WEBGL2=1 -sEMULATE_FUNCTION_POINTER_CASTS=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap']\"")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX .html)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file res")
    endif()
endif()

# Dependencies
include(cmake/CPM.cmake)

# GLFW
if(NOT EMSCRIPTEN)
    CPMAddPackage(
            NAME "GLFW"
            GIT_TAG "3.3.8"
            GITHUB_REPOSITORY "glfw/glfw"
            OPTIONS
            "GLFW_BUILD_DOCS OFF"
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_EXAMPLES OFF"
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
endif()

# ImGui
if(NOT TARGET imgui::imgui)
    CPMFindPackage(
            NAME "imgui"
            GITHUB_REPOSITORY "Les-Informatichiens/imgui-cmake"
            GIT_TAG "v1.90.1-docking-cmake"
            OPTIONS
            "IMGUI_BUILD_OPENGL3_BINDING ON"
    )
endif()
target_sources(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

# ImGuiFileDialog
CPMFindPackage(
        NAME "ImGuiFileDialog"
        GITHUB_REPOSITORY "aiekick/ImGuiFileDialog"
        VERSION "0.6.7"
)
target_link_libraries(${PROJECT_NAME} PRIVATE ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PRIVATE imgui::imgui)

# Engine
target_link_libraries(${PROJECT_NAME} PRIVATE engine)

# Copy resources
message("OUTPUT DIR IS: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(RootFiles "root.config")
set(TestAssets
        "assets/test/teapot.obj"
        "assets/test/cow.obj"
        "assets/test/suzanne.obj"
        "assets/test/textures/default/metallic.png"
        "assets/test/textures/metalgrid/albedo.png"
        "assets/test/textures/metalgrid/normal.png"
        "assets/test/textures/metalgrid/roughness.png"
        "assets/test/textures/metalgrid/metallic.png"
        "assets/test/textures/metalgrid/ao.png"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_basecolor.jpg"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_normal.jpg"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_roughness.jpg"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_metallic.jpg"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_ambientOcclusion.jpg"
        "assets/test/textures/metalgrid/Sci-fi_Wall_011_emissive.jpg"
        "assets/test/textures/rustedmetal/albedo.png"
        "assets/test/textures/rustedmetal/normal.png"
        "assets/test/textures/rustedmetal/roughness.png"
        "assets/test/textures/rustedmetal/metallic.png"
        "assets/test/textures/rustedmetal/ao.png"
        "assets/test/textures/polishedconcrete/albedo.png"
        "assets/test/textures/polishedconcrete/normal.png"
        "assets/test/textures/polishedconcrete/roughness.png"
        "assets/test/textures/polishedconcrete/height.png"

)
copy_assets(RootFiles "." "." CopiedRootFiles)
copy_assets(TestAssets "assets/test" "assets/test" CopiedTestAssets)
set_source_files_properties(${CopiedTestAssets} PROPERTIES GENERATED TRUE)
target_sources(${PROJECT_NAME} PRIVATE ${CopiedRootFiles} ${CopiedTestAssets})
