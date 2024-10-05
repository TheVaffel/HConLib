# HConLib
Utility libraries for C++

## Included components
* FlatAlg - Linear algebra
* HGraf - Graphics utility functions
* HCam - Webcam interface
* Winval - Window system
* Flaudio - Audio engine-ish

## Requirements

* `cmake`
* `g++` (Linux)
* Visual Studio build tools (Windows)

#### For Winval

* X11 (Linux)

#### For Flaudio

* asound (Linux)
* A sane mind, as you could actually hurt your hearing with this. Take care

## Build

* Run `build.sh` (Linux) or `build.bat` (Windows) in root folder

## General Use

#### Linux

You would write something like

`g++ test.cpp -o test -I path/to/HConLib/include -L path/to/HConLib/lib -l FlatAlg`

to compile with FlatAlg etc. Some libraries require further linking, like

`g++ winval_test.cpp -o winval_test -I path/to/HConLib/include -L path/to/HConLib/lib -l Winval -l X11`


#### Windows

The above should work if  `g++` is installed. Equivalents with Visual Studio's compiler (`cl`) would probably work too. Building with `cmake` and `msbuild` might be easier anyway. But I don't judge.

## Examples

* Compile examples by going to examples and run `compile_with_cmake.sh` in Linux or `compile_with_cmake.bat` in Windows
* Supports turbojpeg for image decompression with webcam (see example/CMakeLists.txt to see how it is used)

## Q&A

* **Is this necessary?**
  No, but it makes me feel good about myself

* **May I use it?**
  Yeah, sure

* **Is it optimized?**
  Not very. But hopefully, you are not running this on a toaster
