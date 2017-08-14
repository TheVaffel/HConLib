cmake_minimum_required(VERSION 2.4)
project(HConLib_examples)

add_definitions(-std=c++11)

set(HConLibRoot ${CMAKE_HOME_DIRECTORY}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${HConLibRoot}/build)

set(xcb_libs xcb xcb-image xkbcommon-x11 xkbcommon)
set(vulkan_libs )

set(winval_libs Winval_XCB ${xcb_libs})
set(wingine_libs FlatAlg ${vulkan_libs} Wingine glslang  OSDependent pthread SPIRV SPVRemapper OGLCompiler HLSL vulkan  ${winval_libs})

include_directories(${HConLibRoot}/include)
link_directories(${HConLibRoot}/lib)

#algtest
add_executable(algtest algtest.cpp)
target_link_libraries(algtest FlatAlg)

#lie
add_executable(lie lie.cpp)
target_link_libraries(lie FlatAlg)

find_library(WINVAL_LIB NAMES Winval_XCB PATHS ${HConLibRoot}/lib NO_DEFAULT_PATH)
if(WINVAL_LIB)
	#winval_test
	add_executable(winval_test winval_test.cpp)
	target_link_libraries(winval_test HGraf FlatAlg ${winval_libs})

	#cartoonifycam
	add_executable(cartoonifycam cartoonifycam.cpp)
	target_link_libraries(cartoonifycam ${winval_libs})

	#linetest
	add_executable(linetest linetest.cpp)
	target_link_libraries(linetest HGraf FlatAlg ${winval_libs})

	#hgraftest
	add_executable(hgraftest hgraftest.cpp)
	target_link_libraries(hgraftest HGraf FlatAlg ${winval_libs})

	#xcb_example
	add_executable(xcb_example xcb_example.cpp)
	target_link_libraries(xcb_example ${winval_libs})
	
	#cam
	add_executable(cam cam.cpp)
	target_link_libraries(cam Winval_XCB ${xcb_libs})
  
	find_library(WINGINE_LIB NAMES Wingine PATHS ${HConLibRoot}/lib NO_DEFAULT_PATH)
	if(WINGINE_LIB)
		#vulkan_test
		add_executable(vulkan_test vulkan_test.cpp)
		target_link_libraries(vulkan_test ${wingine_libs})
	endif()
else()
	message("Found no Winval library")
endif()

#flatalg
add_executable(flatalg flatalg.cpp)
target_link_libraries(flatalg FlatAlg)

