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

add_definitions(-std=c++11)
if(NOT WIN32)
  add_definitions(-g -Wall)
endif()
set(includeDir ${CMAKE_HOME_DIRECTORY}/include)
set(srcDir ${CMAKE_HOME_DIRECTORY}/src)

include_directories(${includeDir} )

#FlatAlg
add_library(FlatAlg STATIC ${srcDir}/FlatAlg.cpp)

#HGraf
add_library(HGraf STATIC ${srcDir}/HGraf.cpp)


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

  if(WIN32)
    find_library( VULKAN_LIBS NAMES vulkan-1 HINTS $ENV{VULKAN_SDK}/lib32)
  else()
    find_library(VULKAN_LIBS NAMES vulkan)
  endif()

  if(VULKAN_LIBS)
    #Wingine
    add_library(Wingine STATIC ${srcDir}/Wingine.cpp) # Normal Wingine

    find_library(GLSLANG_LIBS NAMES glslang PATHS ${lib_search_dir} NO_DEFAULT_PATH) #Could search for the rest of the glslang archives too, but takes this as indicator
    if(GLSLANG_LIBS)
      #WingineWithGlslang

      add_library(WingineWithGlslang STATIC ${srcDir}/Wingine.cpp ${srcDir}/glsl_util.cpp) # Wingine with glslang
      if(WIN32)
	       target_compile_definitions(WingineWithGlslang PRIVATE WINGINE_WITH_GLSLANG)
      else()
	       target_compile_definitions(WingineWithGlslang PRIVATE -DWINGINE_WITH_GLSLANG)
      endif()
    else()
      message(">>>glslang is not initialized, Wingine not built")
    endif()
  else()
    message(">>>Vulkan not found, Wingine not built")
  endif()

else()
  if(NOT WIN32)
    message(">>>XLib library not found, Winval not built")
  endif()
endif()
