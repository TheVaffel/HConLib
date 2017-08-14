#!/bin/bash

mkdir build
cd build

#get glslang
git clone git@github.com:KhronosGroup/glslang.git

#build glslang archives
cd glslang 
mkdir build
cd build
cmake ..
make -j $(grep -c ^processor /proc/cpuinfo)

#copy libraries
mkdir ../../../lib
cp $(find . | grep '\.a$') ../../../lib

#copy headers
cd ..
cp -r glslang/ SPIRV/ ../../include/external/vulkan

cd ../..
