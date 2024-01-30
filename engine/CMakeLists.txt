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

# renderer =============================================================================================
target_link_libraries(${PROJECT_NAME} PUBLIC renderer)
# ======================================================================================================

# End dependencies =====================================================================================