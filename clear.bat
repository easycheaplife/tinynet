@echo off
echo ----------------------------------------------------
echo Press any key to delete all files with ending:
echo  *.aps *.idb *.ncp *.obj *.pch *.tmp *.sbr
echo Visual c++/.Net junk 
echo delete file folder use    rd  /S /Q .svn 
echo delete hide file use    del /F /Q /S /A:H-S
echo ----------------------------------------------------

del /F /Q /S *.aps *.idb *.ncp *.obj *.pch *.sbr *.tmp *.pdb *.bsc *.ilk *.res *.ncb *.opt *.suo *.manifest *.dep *.user *.sdf *.sln *.vcxproj *.filters *.cmake *.opensdf *.sdf 
del /F /Q /S /A:H-S tinynet.v11.suo 
rd  /S /Q CMakeFiles Debug lib bin rpcrt4.pdb tinynet.dir x64 Win32
del /F /Q /S CMakeCache.txt

cd srv_test
call clear.bat

cd ..
cd cli_test
call clear.bat
pause