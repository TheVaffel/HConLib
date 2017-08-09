cmake_minimum_required(VERSION 2.4)
project(HConLib)

add_definitions(-std=c++11)
add_library(HConLib src/Wingine.cpp src/glsl_util.cpp src/Winval_XCB.cpp src/HGraf.cpp src/FlatAlg.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

target_include_directories(HConLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
