add_executable(TerrainViewer)

message(STATUS "Creating target 'TerrainViewer'")

set(UI_FILES
    mainwindow.ui
)

set(HEADER_FILES
    mainwindow.h
)

set(SRC_FILES
    main.cpp
    mainwindow.cpp    
)

# Setup filters in Visual Studio
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

target_link_libraries(TerrainViewer
    PRIVATE
    TerrainViewerWidget
)

if(TARGET Qt6::windeployqt)
    add_custom_command(TARGET TerrainViewer
        POST_BUILD
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt6_install_prefix}/bin
        COMMAND Qt6::windeployqt "$<TARGET_FILE_DIR:TerrainViewer>/$<TARGET_FILE_NAME:TerrainViewer>"
    )
endif()
