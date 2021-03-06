# HConLib
Utility libraries for C++

## Included components
* FlatAlg - Linear algebra
* HGraf - Graphics utility functions
* HCam - Webcam interface
* Winval - Window system
* Wingine - Vulkan wrapper dreaming of becoming its own graphics engine some day
* Flaudio - Audio engine-ish

## Requirements

* `cmake`
* `g++` (Linux)
* Visual Studio build tools (Windows)

#### For Winval

* X11 (Linux)

#### For Wingine

* Winval
* Vulkan

The rest concerns KhronoGroup's GLSL-SPIRV translator
* glslang (install script included, requires `git`)
* pthread
* c++11

#### For Flaudio

* asound (Linux)
* A sane mind, as you could actually hurt your hearing with this. Take care

## Build

* Run `build.sh` (Linux) or `build.bat` (Windows) in root folder

#### For Wingine

* Run `initialize_glslang.sh` and make sure Winval builds before running the build script

## General Use

#### Linux

You would write something like

`g++ test.cpp -o test -I path/to/HConLib/include -L path/to/HConLib/lib -l FlatAlg`

to compile with FlatAlg etc. Some libraries require further linking, like

`g++ winval_test.cpp -o winval_test -I path/to/HConLib/include -L path/to/HConLib/lib -l Winval -l X11`

and

`g++ wingine_test.cpp -o wingine_test -I path/to/HConLib/include -L path/to/HConLib/lib -l Winval -l Wingine -l X11 -l glslang -l OSDependent -l pthread -l SPIRV -l SPVRemapper -l OGLCompiler -l HLSL -l vulkan -std=c++11`

You can always see the CMakeLists.txt in the example folder to see what libraries are linked into the mix

#### Windows

The above should work if  `g++` is installed. Equivalents with Visual Studio's compiler (`cl`) would probably work too. Building with `cmake` and `msbuild` might be easier anyway. But I don't judge.

## Examples

* Compile examples by going to examples and run `compile_with_cmake.sh` in Linux or `compile_with_cmake.bat` in Windows
* Expects the Vulkan shared library to be in default linker search path if building with Wingine
* Supports turbojpeg for image decompression with webcam (see example/CMakeLists.txt to see how it is used)

## Q&A

* **Is this necessary?**
  No, but it makes me feel good about myself

* **May I use it?**
  Yeah, sure

* **Is it optimized?**
  Not very. But hopefully, you are not running this on a toaster
