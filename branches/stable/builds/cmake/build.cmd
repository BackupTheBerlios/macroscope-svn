@echo off

set devenv="%VS80COMNTOOLS%..\IDE\devenv.exe"
rem "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"

rem set intel=C:\Program Files (x86)\Intel\Compiler\C++\9.1
rem call "%intel%\IA32\Bin\ICLVars.bat"

rem set cmakeDefs=-DCMAKE_C_COMPILER=icl -DCMAKE_CXX_COMPILER=icl
rem set conf=Debug
rem set cmd=Clean
rem call build-helper.cmd
rem set cmd=Build
rem call build-helper.cmd

rem "%intel%\EM64T\Bin\ICLVars.bat"

rem exit

del /q CMakeCache.txt

set conf=Debug
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=Release
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=MinSizeRel
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=RelWithDebInfo
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd

set x64=-G "Visual Studio 8 2005 Win64"
del /q CMakeCache.txt

set conf=Debug
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=Release
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=MinSizeRel
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
set conf=RelWithDebInfo
set cmd=Clean
call build-helper.cmd
set cmd=Build
call build-helper.cmd
