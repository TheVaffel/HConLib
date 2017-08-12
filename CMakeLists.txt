cmake_minimum_required(VERSION 2.4)
project(HConLib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

add_definitions(-std=c++11)

set(componentNames Winval_XCB Wingine FlatAlg HGraf glsl_util)
set(includeDir ${CMAKE_HOME_DIRECTORY}/include)
set(srcDir ${CMAKE_HOME_DIRECTORY}/src)

set(sourceList)

include_directories(${includeDir} )
link_directories(${srcDir})

foreach(str IN ITEMS ${componentNames})
  
  set(source ${srcDir}/${str}.cpp)

  list(APPEND sourceList ${source})
endforeach(str)

      
#FlatAlg
add_library(FlatAlg STATIC ${srcDir}/FlatAlg.cpp)

#HGraf
add_library(HGraf STATIC ${srcDir}/HGraf.cpp)

#Wingine
add_library(Wingine STATIC ${srcDir}/Wingine.cpp ${srcDir}/glsl_util.cpp)

#Winval_XCB
add_library(Winval_XCB STATIC ${srcDir}/Winval_XCB.cpp)

add_library(HConLib STATIC ${sourceList})
