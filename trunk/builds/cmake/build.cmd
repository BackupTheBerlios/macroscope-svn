@echo off

set devenv="C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.exe"

rem cmake -DCMAKE_BUILD_TYPE=Debug

cmake -DCMAKE_BUILD_TYPE=Release
%devenv% macroscope.sln /Build Release /Project ALL_BUILD.vcproj

rem cmake -DCMAKE_BUILD_TYPE=RelWithDebInf
rem cmake -DCMAKE_BUILD_TYPE=MinSizeRel
rem cmake . -G "Visual Studio 8 2005 Win64"
