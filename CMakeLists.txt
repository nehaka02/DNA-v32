cmake_minimum_required(VERSION 3.16)

project(DNA-v32 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
# find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/gui
)

add_executable(DNA-v32
    src/main.cpp
    src/parseInput.cpp
    src/memory.cpp
    src/cache.cpp
    src/pipeline.cpp
    src/driver.cpp
    include/parseInput.h
    include/memory.h
    include/cache.h
    include/pipeline.h
    include/registers.h
    include/driver.h
    gui/mainwindow.h
    gui/mainwindow.cpp
    gui/mainwindow.ui
)

# target_link_libraries(DNA-v32 PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(DNA-v32 PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS DNA-v32
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(
    ${PROJECT_SOURCE_DIR}/demos/demo_commands2.txt
    ${CMAKE_CURRENT_BINARY_DIR}/demo_commands2.txt
    COPYONLY
)
