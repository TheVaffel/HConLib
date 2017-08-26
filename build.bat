MKDIR build
cd build
cmake ..
MSBUILD ALL_BUILD.vcxproj /P:Configuration=Release
cd ..
