cmake_minimum_required(VERSION 3.16)

project(ReverseHangman LANGUAGES CXX)

# Let Qt handle .ui, moc, and .qrc automatically
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt (Widgets + LinguistTools)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# Translation file (optional)
set(TS_FILES
    ReverseHangman_en_US.ts
)

# All project sources, headers, ui, and the .qrc
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    gameengine.cpp
    gameengine.h

    bodywidget.cpp
    bodywidget.h

    bodyviewdialog.cpp
    bodyviewdialog.h

    Resources.qrc

    ${TS_FILES}
)

# Create the executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ReverseHangman
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    # If you actually use translations, you can enable this:
    # qt_add_translations(ReverseHangman TS_FILES ${TS_FILES})
else()
    add_executable(ReverseHangman
        ${PROJECT_SOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Link against Qt Widgets
target_link_libraries(ReverseHangman PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Optional: nicer bundle settings on macOS / Windows
set_target_properties(ReverseHangman PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# For Qt6, finalize executable (needed with MANUAL_FINALIZATION)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ReverseHangman)
endif()
