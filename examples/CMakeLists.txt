cmake_minimum_required(VERSION 2.4)
project(HConLib_examples)

add_definitions(-std=c++11)

set(HConLibRoot ${CMAKE_HOME_DIRECTORY}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${HConLibRoot}/build)

set(xcb_libs xcb xcb-image xkbcommon-x11 xkbcommon)
set(vulkan_libs vulkan  glslang  SPIRV HLSL OSDependent SPVRemapper pthread OGLCompiler)

set(winval_libs Winval_XCB ${xcb_libs})
set(wingine_libs Wingine FlatAlg ${vulkan_libs} ${winval_libs})

include_directories(${HConLibRoot}/include)
link_directories(${HConLibRoot}/lib)

#algtest
add_executable(algtest algtest.cpp)
target_link_libraries(algtest FlatAlg)

#cam
add_executable(cam cam.cpp)
target_link_libraries(cam Winval_XCB ${xcb_libs})

#lie
add_executable(lie lie.cpp)
target_link_libraries(lie FlatAlg)

#winval_test
add_executable(winval_test winval_test.cpp)
target_link_libraries(winval_test HGraf FlatAlg ${winval_libs})

#cartoonifycam
add_executable(cartoonifycam cartoonifycam.cpp)
target_link_libraries(cartoonifycam ${winval_libs})

#flatalg
add_executable(flatalg flatalg.cpp)
target_link_libraries(flatalg FlatAlg)

#linetest
add_executable(linetest linetest.cpp)
target_link_libraries(linetest HGraf FlatAlg ${winval_libs})

#vulkan_test
add_executable(vulkan_test vulkan_test.cpp)
target_link_libraries(vulkan_test ${wingine_libs})

#hgraftest
add_executable(hgraftest hgraftest.cpp)
target_link_libraries(hgraftest HGraf FlatAlg ${winval_libs})

#xcb_example
add_executable(xcb_example xcb_example.cpp)
target_link_libraries(xcb_example ${winval_libs})
