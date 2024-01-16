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


if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
    message(STATUS "Copying res to build directory")
    file(COPY res DESTINATION "${CMAKE_BINARY_DIR}")
endif ()


# Dependencies ========================================================================================
include(cmake/CPM.cmake)

# Add dependencies here...

# =====================================================================================================


# Compile definitions ==================================================================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __DEBUG__)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RELEASE__ PRIVATE __NDEBUG__)
endif ()
# =====================================================================================================
