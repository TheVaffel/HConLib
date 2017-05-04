#!/bin/bash

currDir="`dirname $0`"
HConLibDir="$currDir/.."
binDir="$currDir/bin"
g++ "$currDir/cam.cpp" -o "$binDir/cam" -I $HConLibDir -lX11
g++ "$currDir/test.cpp" -o "$binDir/test" -I $HConLibDir -lX11
g++ "$currDir/hgraftest.cpp" -o "$binDir/hgraftest" -I $HConLibDir -lX11
g++ "$currDir/linetest.cpp" -o "$binDir/linetest" -I $HConLibDir -lX11
g++ "$currDir/algtest.cpp" -o "$binDir/algtest" -I $HConLibDir -lX11
g++ "$currDir/cartoonifycam.cpp" -o "$binDir/cartoonifycam" -I $HConLibDir -I $currDir -lX11
