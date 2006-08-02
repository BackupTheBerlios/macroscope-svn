@echo off

echo %cmd% configuration %conf% %x64%
if Exist "CMakeCache.txt" goto silentCMake
cmake %cmakeDefs% -DCMAKE_BUILD_TYPE=%conf% %x64%
goto ifErrorCMake
:silentCMake
cmake %cmakeDefs% -DCMAKE_BUILD_TYPE=%conf% %x64% 1>nul
:ifErrorCMake
if errorlevel 0 goto execCmd
echo cmake execution failed
exit 1

:execCmd
%devenv% macroscope.sln /%cmd% %conf% /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makePackage
echo build %conf% configuration failed
exit 1

:makePackage
if "%cmd%" == "Clean" goto exit0
echo Make package %conf%
%devenv% macroscope.sln /%cmd% %conf% /Project PACKAGE.vcproj /Log
if errorlevel 0 goto exit0
echo make debug package failed
exit 1

:exit0
