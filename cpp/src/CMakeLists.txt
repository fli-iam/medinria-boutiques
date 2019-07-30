cmake_minimum_required(VERSION 3.1.0)

project(BoutiquesGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(${PROJECT_NAME}
    mainwindow.cpp
    searchtoolswidget.cpp
    invocationguiwidget.cpp
    invocationwidget.cpp
    executionwidget.cpp
    toolboxwidget.cpp
    dropwidget.cpp
    abstractfilehandler.cpp
    filehandler.cpp
    main.cpp
)

target_link_libraries(${PROJECT_NAME} 
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  )
