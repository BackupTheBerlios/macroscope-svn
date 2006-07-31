@echo off

set devenv="C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"
set cmd=Rebuild

call build-helper.cmd
rem set x64=-G "Visual Studio 8 2005 Win64"
rem set cmd=Rebuild
rem del CMakeCache.txt
rem call build-helper.cmd
