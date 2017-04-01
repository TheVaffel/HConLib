#!/bin/bash

currdir="`dirname $0`"
HConLibDir="$currdir/.."

g++ "$currdir/cam.cpp" -o "$currdir/cam" -I $HConLibDir -lX11
g++ "$currdir/test.cpp" -o "$currdir/test" -I $HConLibDir -lX11
g++ "$currdir/hgraftest.cpp" -o "$currdir/test" -I $HConLibDir -lX11
g++ "$currdir/linetest.cpp" -o "$currdir/linetest" -I $HConLibDir -lX11
g++ "$currdir/algtest.cpp" -o "$currdir/algtest" -I $HConLibDir -lX11
