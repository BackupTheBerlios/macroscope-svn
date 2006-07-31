@echo off

set devenv="C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"
set cmd=Build

set conf=Debug
call build-helper.cmd
set conf=Release
call build-helper.cmd
set conf=MinSizeRel
call build-helper.cmd
set conf=RelWithDebInfo
call build-helper.cmd

set x64=-G "Visual Studio 8 2005 Win64"
set cmd=Rebuild
del CMakeCache.txt

set conf=Debug
call build-helper.cmd
set conf=Release
call build-helper.cmd
set conf=MinSizeRel
call build-helper.cmd
set conf=RelWithDebInfo
call build-helper.cmd
