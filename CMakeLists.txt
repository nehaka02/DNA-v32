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

add_executable(DNA-v32
  main.cpp
  parseInput.h parseInput.cpp
  memory.h memory.cpp
  cache.h cache.cpp
  pipeline.h pipeline.cpp
  registers.h
  driver.cpp driver.h
  mainwindow.h mainwindow.cpp mainwindow.ui
  assembler.py
  mnemonic_table.py

)
# target_link_libraries(DNA-v32 PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(DNA-v32 PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS DNA-v32
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
