@echo off

set devenv="%VS80COMNTOOLS%..\IDE\devenv.exe"
rem "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"

set cmakeDefs=-DPRIVATE_RELEASE=1

rem goto test

del /q CMakeCache.txt include\adicpp\config.h *.sln *.ncb
rmdir /s /q CMakeFiles
rmdir /s /q ALL_BUILD.dir
rmdir /s /q PACKAGE.dir
rmdir /s /q ZERO_CHECK.dir
rmdir /s /q _CPack_Packages

set conf=Debug
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=Release
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=MinSizeRel
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=RelWithDebInfo
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd

set generator=-G "Visual Studio 8 2005 Win64"
del /q CMakeCache.txt include\adicpp\config.h *.sln *.ncb
rmdir /s /q CMakeFiles
rmdir /s /q ALL_BUILD.dir
rmdir /s /q PACKAGE.dir
rmdir /s /q ZERO_CHECK.dir
rmdir /s /q _CPack_Packages

set conf=Debug
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=Release
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=MinSizeRel
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd
set conf=RelWithDebInfo
set cmd=Clean
call builds\cmake\build-helper.cmd
set cmd=Build
call builds\cmake\build-helper.cmd

goto exit0

:test
set intel=C:\Program Files (x86)\Intel\Compiler\C++\9.1
call "%intel%\IA32\Bin\ICLVars.bat"

set generator=-G "NMake Makefiles"
set cmakeDefs=-DCMAKE_C_COMPILER=icl -DCMAKE_CXX_COMPILER=icl

set conf=Debug
set cmd=NMAKE
call builds\cmake\build-helper.cmd
nmake all

rem "%intel%\EM64T\Bin\ICLVars.bat"

:exit0
