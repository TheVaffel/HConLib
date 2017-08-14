cmake_minimum_required(VERSION 3.0)
project(HConLib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

add_definitions(-std=c++11)
set(includeDir ${CMAKE_HOME_DIRECTORY}/include)
set(srcDir ${CMAKE_HOME_DIRECTORY}/src)

include_directories(${includeDir} )

#FlatAlg
add_library(FlatAlg STATIC ${srcDir}/FlatAlg.cpp)

#HGraf
add_library(HGraf STATIC ${srcDir}/HGraf.cpp)

find_library( XCB_LIBS NAMES xcb xcb-image xkbcommon-x11 xkbcommon oeuathnt)
if(XCB_LIBS)
	#Winval_XCB
	add_library(Winval_XCB STATIC ${srcDir}/Winval_XCB.cpp)

	find_library( VULKAN_LIBS NAMES vulkan)
	
	if(VULKAN_LIBS)
		#Wingine
		add_library(Wingine STATIC ${srcDir}/Wingine.cpp ${srcDir}/glsl_util.cpp)
	else()
		message("Vulkan not found, Wingine not built")
	endif()

else()
	message("Not all required XCB libraries found, Winval not built")
endif()

