@echo off
::	for comment
cmake -G "Visual Studio 11" .
msbuild.exe ALL_BUILD.vcxproj