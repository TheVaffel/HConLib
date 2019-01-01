#!/bin/bash

mkdir build 2> /dev/null
cd build

#get glslang
git clone https://github.com/KhronosGroup/glslang.git

#build glslang archives
cd glslang 
git checkout vulkan-1.1-rc9

mkdir build 2> /dev/null
cd build
cmake ..
make -j $(grep -c ^processor /proc/cpuinfo)

#copy libraries
mkdir ../../../lib 2> /dev/null
cp $(find . | grep '\.a$') ../../../lib

#copy headers
cd ..
cp -r glslang/ SPIRV/ ../../include/external/vulkan

cd ../..
