add_executable(TerrainViewer)

message(STATUS "Creating target 'TerrainViewer'")

# Files for the project
set(RESOURCE_FILES mainwindow.qrc)

set(UI_FILES
    mainwindow.ui
    openterraindialog.ui
    parameterdock.ui
)

set(HEADER_FILES
    camera.h
    mainwindow.h
    occlusion.h
    openterraindialog.h
    parameterdock.h
    terrain.h
    terrainimages.h
    terrainviewerparameters.h
    terrainviewerwidget.h
    tessellation_utils.h
    utils.h
    watersimulation.h
)

set(SRC_FILES
    camera.cpp
    main.cpp
    mainwindow.cpp
    occlusion.cpp
    openterraindialog.cpp
    parameterdock.cpp
    terrain.cpp
    terrainimages.cpp
    terrainviewerwidget.cpp
    tessellation_utils.cpp
    watersimulation.cpp
)

# Setup filters in Visual Studio
source_group("Resources" FILES ${RESOURCE_FILES})
source_group("Forms" FILES ${UI_FILES})
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SRC_FILES})

# Copy the ReSharper config file
configure_file(
    "${CMAKE_SOURCE_DIR}/TerrainViewer.sln.DotSettings"
    ${CMAKE_BINARY_DIR} COPYONLY
)

target_sources(TerrainViewer
    PUBLIC
    ${HEADER_FILES}
    PRIVATE
    ${SRC_FILES}
    ${RESOURCE_FILES}
    ${UI_FILES}
)

target_include_directories(TerrainViewer
    PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(TerrainViewer
    PRIVATE
    OpenMP::OpenMP_CXX
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${OpenCV_LIBS}
)

if(TARGET Qt6::windeployqt)
    add_custom_command(TARGET TerrainViewer
        POST_BUILD
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt6_install_prefix}/bin
        COMMAND Qt6::windeployqt "$<TARGET_FILE_DIR:TerrainViewer>/$<TARGET_FILE_NAME:TerrainViewer>"
    )
endif()
