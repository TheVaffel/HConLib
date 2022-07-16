cmake_minimum_required(VERSION 3.0)
project(HConLib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

# set(DEBUG true)

if(WIN32)
  if(DEBUG)
    set(lib_search_dir ${CMAKE_HOME_DIRECTORY}/lib/Debug)
  else()
    set(lib_search_dir ${CMAKE_HOME_DIRECTORY}/lib/Release)
  endif()
else()
  set(lib_search_dir ${CMAKE_HOME_DIRECTORY}/lib)
endif()

add_definitions(-std=c++2a -mavx -g)

if(NOT WIN32)
  add_definitions(-g -Wall -fPIC -Wno-sign-compare)
endif()
set(includeDir ${CMAKE_HOME_DIRECTORY}/include  ${CMAKE_HOME_DIRECTORY}/../flawed/include)
set(srcDir ${CMAKE_HOME_DIRECTORY}/src)

include_directories(${includeDir} )

#FlatAlg
add_library(FlatAlg STATIC ${srcDir}/FlatAlg.cpp)

#HGraf
add_library(HGraf STATIC
  ${srcDir}/HGraf/primitives.cpp
  ${srcDir}/HGraf/triangulation.cpp
  ${srcDir}/HGraf/draw.cpp
  ${srcDir}/HGraf/mesh.cpp
  ${srcDir}/HGraf/bvh.cpp
  ${srcDir}/HGraf/improc_ssim.cpp)

#HCam
add_library(HCam STATIC ${srcDir}/HCam.cpp)

#Flaudio
if(NOT WIN32)
  find_library(FLAUDIO_LIB NAMES asound)
else()
  set(FLAUDIO_LIB TRUE)
endif()

if(FLAUDIO_LIB)
  add_library(Flaudio STATIC ${srcDir}/Flaudio.cpp)
else()
  message(">>>Library asound not found, not building Flaudio")
endif()

if(NOT WIN32)
  find_library(WINVAL_LIB NAMES X11)
else()
  set(WINVAL_LIB TRUE)
endif()

if(WINVAL_LIB)
  #Winval
  add_library(Winval STATIC ${srcDir}/Winval.cpp)
else()
  if(NOT WIN32)
    message(">>>XLib library not found, Winval not built")
  endif()
endif()


# Tests

set(FLAWED_PATH ${CMAKE_HOME_DIRECTORY}/../flawed)
set(FLAWED_OUTPUT_PATH flawed_tests)

include(${FLAWED_PATH}/FlawedTest.cmake)

flawed_add_test_utils(${srcDir}/HGraf/test_utils.cpp)

flawed_add_tests(
  ${srcDir}/HGraf/improc.test.cpp
  ${srcDir}/HGraf/improc_filtering.test.cpp
  ${srcDir}/HGraf/improc_io.test.cpp
  ${srcDir}/HGraf/improc_diff.test.cpp)

flawed_write_test_target(OpenImageIO OpenImageIO_Util HGraf)
