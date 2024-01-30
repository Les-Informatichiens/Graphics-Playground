cmake_minimum_required(VERSION 3.26)
project(engine VERSION 0.1)

add_library(
        ${PROJECT_NAME} STATIC
        src/engine/engine.cpp
        include/engine/engine.h
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# dependencies =========================================================================================

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
    target_link_libraries(${PROJECT_NAME} PRIVATE libglew_static)
endif ()
# =====================================================================================================

# renderer =============================================================================================
target_link_libraries(${PROJECT_NAME} PUBLIC renderer)
# ======================================================================================================

# End dependencies =====================================================================================