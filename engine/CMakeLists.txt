cmake_minimum_required(VERSION 3.26)
project(engine VERSION 0.1)

add_library(
        ${PROJECT_NAME} STATIC
        src/engine/engine.cpp
        include/engine/engine.h
        include/engine/graphics/Renderer.h
        src/engine/graphics/Renderer.cpp
        include/engine/EngineInstance.h
        src/engine/EngineInstance.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# dependencies =========================================================================================

# glm =================================================================================================
CPMAddPackage(
        NAME "glm"
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "0.9.9.8"
)

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
# =====================================================================================================

# renderer =============================================================================================
target_link_libraries(${PROJECT_NAME} PUBLIC graphicsAPI)
# ======================================================================================================

# End dependencies =====================================================================================