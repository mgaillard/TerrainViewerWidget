add_library(TerrainViewerWidget STATIC)

message(STATUS "Creating target 'TerrainViewerWidget'")

# Files for the project
set(RESOURCE_FILES resources/terrainviewerwidget.qrc)

set(UI_FILES
    source/openterraindialog.ui
    source/parameterdock.ui
)

set(HEADER_FILES
    include/camera.h
    include/occlusion.h
    include/openterraindialog.h
    include/parameterdock.h
    include/terrain.h
    include/terrainimages.h
    include/terrainviewerparameters.h
    include/terrainviewerwidget.h
    include/tessellation_utils.h
    include/utils.h
    include/watersimulation.h
)

set(SRC_FILES
    source/camera.cpp
    source/occlusion.cpp
    source/openterraindialog.cpp
    source/parameterdock.cpp
    source/terrain.cpp
    source/terrainimages.cpp
    source/terrainviewerwidget.cpp
    source/tessellation_utils.cpp
    source/watersimulation.cpp
)

# Setup filters in Visual Studio
source_group("Forms" FILES ${UI_FILES})
source_group("Resources" FILES ${RESOURCE_FILES})
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SRC_FILES})

target_sources(TerrainViewerWidget
    PUBLIC
    ${HEADER_FILES}
    ${RESOURCE_FILES}
    PRIVATE
    ${SRC_FILES}
    ${UI_FILES}
)

target_include_directories(TerrainViewerWidget
    PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(TerrainViewerWidget 
    PUBLIC
    OpenMP::OpenMP_CXX
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${OpenCV_LIBS}
)
