cmake_minimum_required(VERSION 3.18.0)

project(TerrainViewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activate OpenMP
find_package(OpenMP REQUIRED)

# Add CMake recipes
list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# Dependencies for application
include(opencv)
include(qt6)

add_subdirectory(TerrainViewerWidget)
add_subdirectory(TerrainViewer)

# Set the project as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TerrainViewer)
