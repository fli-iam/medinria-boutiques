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
    main.cpp
)

target_link_libraries(${PROJECT_NAME} 
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  )






# project(BoutiquesGUI)

# cmake_minimum_required(VERSION 2.8)

# find_package(Qt5 REQUIRED COMPONENTS
#   Core
#   Gui
#   Widgets
#   )

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# #   Find includes in corresponding build (and source) directories
# #   (nedded for generated files)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

# include_directories(
#   ThirdParts
#   ThirdParts/APIs
#   ThirdParts/APIs/SoapNative
#   )

# set(SRC_LIST
#   main.cpp
#   )

# add_executable(${PROJECT_NAME}
#   ${SRC_LIST}
#   )

# target_link_libraries(${PROJECT_NAME}
#   Qt5::Core
#   Qt5::Gui
#   Qt5::Widgets
#   )
