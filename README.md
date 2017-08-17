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

#### For Winval

* X11

#### For Wingine

* Winval
* Vulkan
The rest concerns KhronoGroup's GLSL-SPIRV translator
* glslang (install script included, requires `git`)
* pthread
* c++11

#### For Flaudio

* asound

## Build

* Run `build.sh` script in root folder

#### For Winval

* Might need to install xcb libraries, run the `install_xcb.sh` before `build.sh`

#### For Wingine

* Run `initialize_glslang.sh` and make sure Winval builds before running `build.sh`

## General Use

You would write something like

`g++ test.cpp -o test -I path/to/HConLib/include -L path/to/HConLib/lib -l FlatAlg`

to compile with FlatAlg etc. Some libraries require further linking, like

`g++ winval_test.cpp -o winval_test -I path/to/HConLib/include -L path/to/HConLib/lib -l Winval -l X11`

and

`g++ wingine_test.cpp -o wingine_test -I path/to/HConLib/include -L path/to/HConLib/lib -l Winval -l Wingine -l X11 -l glslang -l OSDependent -l pthread -l SPIRV -l SPVRemapper -l OGLCompiler -l HLSL -l vulkan -std=c++11`

You can always see the CMakeLists.txt in the example folders to see what libraries are linked with

## Examples

* Compile examples by going to examples and run `compile_with_cmake.sh`
* Expects the Vulkan shared library to be in default linker search path if building with Wingine
* Supports turbojpeg for image decompression with webcam (see example/CMakeLists.txt to see how it is used)

## Q&A

* **Is this necessary?**
  No, but it makes me feel good about myself

* **May I use it?**
  Yeah, sure

* **Is it optimized?**
  Not very. But hopefully, you are not running this on a toaster
