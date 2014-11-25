@echo off
::	for comment
cmake -G "Visual Studio 11 Win64" .
msbuild.exe ALL_BUILD.vcxproj
