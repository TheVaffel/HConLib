cmake_minimum_required(VERSION 2.4)
project(HConLib_examples)

add_definitions(-std=c++11)

if(NOT WIN32)
  add_definitions(-Wall -g)
endif()

set(HConLibRoot ${CMAKE_HOME_DIRECTORY}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

if(WIN32)
  link_directories($ENV{VULKAN_SDK}/lib32)
endif()

#set(DEBUG true)

if(WIN32)
  if(DEBUG)
    set(lib_search_path ${HConLibRoot}/lib/Debug)
  else()
    set(lib_search_path ${HConLibRoot}/lib/Release)
  endif()
  set(winval_libs Winval)
  set(flaudio_libs Flaudio)
else()
  set(lib_search_path ${HConLibRoot}/lib)
  set(x_libs X11)
  set(winval_libs Winval ${x_libs})
  set(flaudio_libs Flaudio asound)
endif()

if(WIN32)
  set(VULKAN_LIB_NAME vulkan-1)
else()
  set(VULKAN_LIB_NAME vulkan)
endif()

set(wingine_libs FlatAlg ${vulkan_libs} Wingine glslang  OSDependent SPIRV SPVRemapper OGLCompiler HLSL ${VULKAN_LIB_NAME} ${winval_libs})
if(WIN32)
  set(wingine_libs ${wingine_libs})
else()
  set(wingine_libs ${wingine_libs} pthread)
endif()

include_directories(${HConLibRoot}/include)
link_directories(${HConLibRoot}/lib)

#algtest
add_executable(algtest algtest.cpp)
target_link_libraries(algtest FlatAlg)

#lie
add_executable(lie lie.cpp)
target_link_libraries(lie FlatAlg)

#flatalg
add_executable(flatalg flatalg.cpp)
target_link_libraries(flatalg FlatAlg)

find_library(FLAUDIO_LIB NAMES Flaudio PATHS ${lib_search_path} NO_DEFAULT_PATH)

if(FLAUDIO_LIB)
  #audio
  add_executable(audio audio.cpp)
  target_link_libraries(audio ${flaudio_libs})
else()
  message(">>>Did not find Flaudio, audio not built")
endif()

find_library(WINVAL_LIB NAMES Winval PATHS ${lib_search_path} NO_DEFAULT_PATH)
if(WINVAL_LIB)
	#winval_test
	add_executable(winval_test winval_test.cpp)
	target_link_libraries(winval_test HGraf FlatAlg ${winval_libs})

	#linetest
	add_executable(linetest linetest.cpp)
	target_link_libraries(linetest HGraf FlatAlg ${winval_libs})

	#hgraftest
	add_executable(hgraftest hgraftest.cpp)
	target_link_libraries(hgraftest HGraf FlatAlg ${winval_libs})

	#colors
	add_executable(colors colors.cpp)
	target_link_libraries(colors ${winval_libs})

  if(FLAUDIO_LIB)
  	#piano
  	add_executable(piano piano.cpp)
  	target_link_libraries(piano ${flaudio_libs} ${winval_libs})
  endif()

  find_library(HCAM_LIB NAMES HCam PATHS ${lib_search_path} NO_DEFAULT_PATH)
  if(HCAM_LIB)
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
  else()
    message(">>>HCam not found, not building HCam dependent binaries")
  endif()

	find_library(WINGINE_LIB NAMES Wingine PATHS ${lib_search_path} NO_DEFAULT_PATH)
	if(WINGINE_LIB)
		#vulkan_test
		add_executable(vulkan_test vulkan_test.cpp)
		target_link_libraries(vulkan_test ${wingine_libs})

    #light_test
    add_executable(light_test light_test.cpp)
    target_link_libraries(light_test ${wingine_libs})
  else()
    message(">>>Found no Wingine library, not building Wingine dependent binaries")
	endif()
else()
	message(">>>Found no Winval library, not building Winval dependent binaries")
endif()
