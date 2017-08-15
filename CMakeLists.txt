cmake_minimum_required(VERSION 3.0)
project(HConLib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)

add_definitions(-std=c++11 -g)
set(includeDir ${CMAKE_HOME_DIRECTORY}/include)
set(srcDir ${CMAKE_HOME_DIRECTORY}/src)

include_directories(${includeDir} )

#FlatAlg
add_library(FlatAlg STATIC ${srcDir}/FlatAlg.cpp)

#HGraf
add_library(HGraf STATIC ${srcDir}/HGraf.cpp)

#HCam
add_library(HCam STATIC ${srcDir}/HCam.cpp)

find_library(ALSA_LIB NAMES asound)
if(ALSA_LIB)
  #Flaudio
  add_library(Flaudio STATIC ${srcDir}/Flaudio.cpp)
else()
  message(">>>Library asound not found, not building Flaudio")
endif()

find_library(XLIB_LIB NAMES X11)

if(XLIB_LIB)
	#Winval
	add_library(Winval STATIC ${srcDir}/Winval.cpp)

	find_library( VULKAN_LIBS NAMES vulkan)
	
	if(VULKAN_LIBS)	
		find_library(GLSLANG_LIBS NAMES glslang PATHS ${CMAKE_HOME_DIRECTORY}/lib NO_DEFAULT_PATH) #Could search for the rest of the glslang archives too, but takes this as indicator
		
		if(GLSLANG_LIBS)
			#Wingine
			add_library(Wingine STATIC ${srcDir}/Wingine.cpp ${srcDir}/glsl_util.cpp)
		else()
			message(">>>glslang is not initialized, Wingine not built")
		endif()
	else()
		message(">>>Vulkan not found, Wingine not built")
	endif()

else()
	message(">>>XLib library not found, Winval not built")
endif()

