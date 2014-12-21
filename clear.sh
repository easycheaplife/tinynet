#!/bin/bash
echo 'start clear'
rm -rf CMakeFiles
rm -f cmake_install.cmake
rm -f CMakeCache.txt
rm -f Makefile
rm -rf lib
rm -rf cli_test/CMakeFiles
rm -f cli_test/cmake_install.cmake
rm -f cli_test/Makefile
rm -rf srv_test/CMakeFiles
rm -f srv_test/cmake_install.cmake
rm -f srv_test/Makefile

echo 'end clear'
