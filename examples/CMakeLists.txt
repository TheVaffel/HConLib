cmake_minimum_required(VERSION 2.4)
project(HConLib_examples)

add_definitions(-std=c++11 -Wall -g)

set(HConLibRoot ${CMAKE_HOME_DIRECTORY}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${HConLibRoot}/build)

set(x_libs X11)
set(vulkan_libs )

set(winval_libs Winval ${x_libs})
set(wingine_libs FlatAlg ${vulkan_libs} Wingine glslang  OSDependent pthread SPIRV SPVRemapper OGLCompiler HLSL vulkan  ${winval_libs})

include_directories(${HConLibRoot}/include)
link_directories(${HConLibRoot}/lib)

#algtest
add_executable(algtest algtest.cpp)
target_link_libraries(algtest FlatAlg)

#lie
add_executable(lie lie.cpp)
target_link_libraries(lie FlatAlg)

find_library(FLAUDIO_LIB NAMES Flaudio PATHS ${HConLibRoot}/lib NO_DEFAULT_PATH)

if(FLAUDIO_LIB)
  #audio
  add_executable(audio audio.cpp)
  target_link_libraries(audio Flaudio asound)
else()
  message(">>>Did not find Flaudio, audio not built")
endif()

find_library(WINVAL_LIB NAMES Winval PATHS ${HConLibRoot}/lib NO_DEFAULT_PATH)
if(WINVAL_LIB)
	#winval_test
	add_executable(winval_test winval_test.cpp)
	target_link_libraries(winval_test HGraf FlatAlg ${winval_libs})

	#piano
	add_executable(piano piano.cpp)
	target_link_libraries(piano Flaudio asound ${winval_libs})

	#linetest
	add_executable(linetest linetest.cpp)
	target_link_libraries(linetest HGraf FlatAlg ${winval_libs})

	#hgraftest
	add_executable(hgraftest hgraftest.cpp)
	target_link_libraries(hgraftest HGraf FlatAlg ${winval_libs})

	#colors
	add_executable(colors colors.cpp)
	target_link_libraries(colors ${winval_libs})
	
	#cam
	add_executable(cam cam.cpp)
	target_link_libraries(cam HCam ${winval_libs})

	#cartoonifycam
	add_executable(cartoonifycam cartoonifycam.cpp)
	target_link_libraries(cartoonifycam HCam ${winval_libs})
  
	find_library(TURBO_JPEG_LIB NAMES turbojpeg)
	if(TURBO_JPEG_LIB)
		message(">>>Found turbojpeg, compiling cam.cpp with turbo support")
		target_compile_definitions(cam PRIVATE USE_TURBOJPEG)
		target_link_libraries(cam turbojpeg)
		
		target_compile_definitions(cartoonifycam PRIVATE USE_TURBOJPEG)
		target_link_libraries(cartoonifycam turbojpeg)
	else()
		message(">>>Did not find turbojpeg, compiling cam.cpp without it")
	endif()


	find_library(WINGINE_LIB NAMES Wingine PATHS ${HConLibRoot}/lib NO_DEFAULT_PATH)
	if(WINGINE_LIB)
		#vulkan_test
		add_executable(vulkan_test vulkan_test.cpp)
		target_link_libraries(vulkan_test ${wingine_libs})
	endif()
else()
	message(">>>Found no Winval library, not building Winval dependent binaries")
endif()

#flatalg
add_executable(flatalg flatalg.cpp)
target_link_libraries(flatalg FlatAlg)


