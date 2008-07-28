@echo off

echo %cmd% configuration %conf% %generator%
if Exist "CMakeCache.txt" goto silentCMake
cmake %cmakeDefs% -DCMAKE_BUILD_TYPE=%conf% %generator% .
goto ifErrorCMake
:silentCMake
cmake %cmakeDefs% -DCMAKE_BUILD_TYPE=%conf% %generator% . 1>nul
:ifErrorCMake
if errorlevel 0 goto execCmd
echo cmake execution failed
exit 1

:execCmd
if /i "%cmd%" == "NMAKE" goto exit0

%devenv% macroscope.sln /%cmd% %conf% /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makePackage
echo build %conf% configuration failed
exit 1

:makePackage
if /i "%cmd%" == "Clean" goto exit0
echo Make package %conf%
%devenv% macroscope.sln /%cmd% %conf% /Project PACKAGE.vcproj /Log
if errorlevel 0 goto exit0
echo make debug package failed
exit 1

:exit0
