@echo off

echo Build configuration %conf% %x64%
cmake -DCMAKE_BUILD_TYPE=%conf% %x64%
if errorlevel 0 goto build
echo cmake execution failed
exit 1

:build
%devenv% macroscope.sln /%cmd% %conf% /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makePackage
echo build %conf% configuration failed
exit 1

:makePackage
echo Make package %conf%
%devenv% macroscope.sln /%cmd% %conf% /Project PACKAGE.vcproj /Log
if errorlevel 0 goto exit0
echo make debug package failed
exit 1

:exit0
