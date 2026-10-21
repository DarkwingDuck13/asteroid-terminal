cmake_minimum_required(VERSION 3.16)
project(asteroid-terminal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

set(SOURCES
    src/main.cpp
    src/ptyterminal.cpp
    src/ptyterminal.h
    src/resources.qrc
)

add_executable(asteroid-terminal ${SOURCES})

target_link_libraries(asteroid-terminal PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    util
)

install(TARGETS asteroid-terminal RUNTIME DESTINATION bin)
install(FILES asteroid-terminal.desktop DESTINATION share/applications)