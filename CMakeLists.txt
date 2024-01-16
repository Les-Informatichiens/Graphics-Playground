cmake_minimum_required(VERSION 3.26)
project(graphics-playground VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

add_executable(
        ${PROJECT_NAME}
        src/main.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
)

if (MSVC)
    # Set the app as solution startup project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # Parallel c++ compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()


if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-s WASM=1 -sUSE_GLFW=3 -sUSE_WEBGL2 -sEMULATE_FUNCTION_POINTER_CASTS=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap']\"")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX .html)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        set(CMAKE_CXX_FLAGS  "--preload-file res")
    endif ()
endif()

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
if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGL2=1")
else()
    if(NOT TARGET libglew_static)
        CPMAddPackage(
                NAME "GLEW"
                GITHUB_REPOSITORY "Perlmint/glew-cmake"
                GIT_TAG "glew-cmake-2.2.0"
        )
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE libglew_static)
endif()
# =====================================================================================================

# glfw ================================================================================================
if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_GLFW=3")
endif()
# =====================================================================================================

# imgui ===============================================================================================
if (NOT TARGET imgui::imgui)
    CPMFindPackage(
            NAME "imgui"
            GITHUB_REPOSITORY "Les-Informatichiens/imgui-cmake"
            GIT_TAG "v1.90.1-docking-cmake"
            OPTIONS
            "IMGUI_BUILD_OPENGL3_BINDING ON"
            "IMGUI_BUILD_GLFW_BINDING ON"
    )
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
# =====================================================================================================

# =====================================================================================================


# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================
