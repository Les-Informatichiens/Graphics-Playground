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
        src/application.h
        src/imgui/ImGuiRenderer.cpp
        src/imgui/ImGuiRenderer.h
        src/imgui/ImGuiInstance.cpp
        src/imgui/ImGuiInstance.h
        src/imgui/drawingPanel/shape.h
        src/imgui/drawingPanel/picasso.h
        src/imgui/ImGuiContextAdapter.cpp
        src/imgui/scenePanel/SceneEditor.cpp
        src/imgui/scenePanel/SceneEditor.h
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
)

if (MSVC)
    # Set the app as solution startup project
    set_property(DIRECTORY ../.. PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # Parallel c++ compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()


if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-s WASM=1 -sUSE_GLFW=3 -sUSE_WEBGL2 -sEMULATE_FUNCTION_POINTER_CASTS=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap']\"")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX .html)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        set(CMAKE_CXX_FLAGS "--preload-file res")
    endif ()
endif ()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
    message(STATUS "Copying res to build directory")
    file(COPY res DESTINATION "${CMAKE_BINARY_DIR}")
endif ()


# Dependencies ========================================================================================
include(cmake/CPM.cmake)

## glew ================================================================================================
#if (EMSCRIPTEN)
#    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGL2=1")
#else ()
#    CPMAddPackage(
#            NAME "GLEW"
#            GITHUB_REPOSITORY "Perlmint/glew-cmake"
#            GIT_TAG "glew-cmake-2.2.0"
#            OPTIONS
#            "glew-cmake_BUILD_SHARED OFF"
#            "glew-cmake_BUILD_STATIC ON"
#            "glew-cmake_ONLY_LIBS ON"
#    )
#    target_link_libraries(${PROJECT_NAME} PRIVATE libglew_static)
#endif ()
## =====================================================================================================

# glfw ================================================================================================
if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_GLFW=3")
else ()
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
endif ()

# =====================================================================================================

# imgui ===============================================================================================
if (NOT TARGET imgui::imgui)
    CPMFindPackage(
            NAME "imgui"
            GITHUB_REPOSITORY "Les-Informatichiens/imgui-cmake"
            GIT_TAG "v1.90.1-docking-cmake"
            OPTIONS
            "IMGUI_BUILD_OPENGL3_BINDING ON"
    )
endif ()
target_sources(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
# ======================================================================================================

# ImGuiFileDialog ======================================================================================
CPMFindPackage(
        NAME "ImGuiFileDialog"
        GITHUB_REPOSITORY "aiekick/ImGuiFileDialog"
        VERSION "0.6.7"
)
target_link_libraries(${PROJECT_NAME} PRIVATE ImGuiFileDialog)

target_link_libraries(ImGuiFileDialog PRIVATE imgui::imgui)
# ======================================================================================================

# engine ===============================================================================================

target_link_libraries(${PROJECT_NAME} PRIVATE engine)

# ======================================================================================================


# End dependencies =====================================================================================


# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================


# Copy resources to build directory ===================================================================

set(
        RootFiles
        "root.config"
)

set(
        TestAssets
        "assets/test/teapot.obj"
        "assets/test/cow.obj"
        "assets/test/suzanne.obj"

        "assets/test/textures/default/metallic.png"

        "assets/test/textures/metalgrid/albedo.png"
        "assets/test/textures/metalgrid/normal.png"
        "assets/test/textures/metalgrid/roughness.png"
        "assets/test/textures/metalgrid/metallic.png"
        "assets/test/textures/metalgrid/ao.png"
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

message("OUTPUT DIR IS: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

copy_assets(RootFiles "." "." CopiedRootFiles)
copy_assets(TestAssets "assets/test" "assets/test" CopiedTestAssets)

set_source_files_properties(${CopiedTestAssets} PROPERTIES GENERATED TRUE)
target_sources(${PROJECT_NAME} PRIVATE ${CopiedRootFiles} ${CopiedTestAssets})