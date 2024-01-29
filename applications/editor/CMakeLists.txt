cmake_minimum_required(VERSION 3.26)
project(editor VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

add_executable(
        ${PROJECT_NAME}
        src/editor.cpp
        ../application.cpp
        ../application.h
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

# glm =================================================================================================
CPMAddPackage(
        NAME "glm"
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "0.9.9.8"
)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
# =====================================================================================================

# glew ================================================================================================
if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGL2=1")
else ()
    CPMFindPackage(
            NAME "GLEW"
            GITHUB_REPOSITORY "Perlmint/glew-cmake"
            GIT_TAG "glew-cmake-2.2.0"
            OPTIONS
            "glew-cmake_BUILD_SHARED OFF"
            "glew-cmake_BUILD_STATIC ON"
            "glew-cmake_ONLY_LIBS ON"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE libglew_static)
endif ()
# =====================================================================================================

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

# End dependencies =====================================================================================


# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================