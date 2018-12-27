mkdir build
cd build

set conf=Release

git clone https://github.com/KhronosGroup/glslang.git

cd glslang
git checkout vulkan-1.1-rc9
mkdir build
cd build
cmake ..
msbuild ALL_BUILD.vcxproj /P:Configuration=%conf%

mkdir ..\..\..\lib
for /f "usebackq delims=" %%a in (`dir /s /b *.lib`) do copy "%%a" ..\..\..\lib\%conf% /Y

#copy headers
cd ..
xcopy glslang /h /e ..\..\include\external\vulkan\glslang /i
xcopy SPIRV /h /e ..\..\include\external\vulkan\SPIRV /i

cd ..\..
