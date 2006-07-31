@echo off

echo Build configuration Debug %x64%
cmake -DCMAKE_BUILD_TYPE=Debug %x64%
if errorlevel 0 goto buildDebug
echo cmake execution failed
exit 1
:buildDebug
%devenv% macroscope.sln /%cmd% Debug /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makeDebugPackage
echo build debug configuration failed
exit 1
:makeDebugPackage
echo Make package Debug
%devenv% macroscope.sln /%cmd% Debug /Project PACKAGE.vcproj /Log
if errorlevel 0 goto makeRelease
echo make debug package failed
exit 1

:makeRelease
echo Build configuration Release %x64%
cmake -DCMAKE_BUILD_TYPE=Release %x64%
if errorlevel 0 goto buildRelease
echo cmake execution failed
exit 1
:buildRelease
%devenv% macroscope.sln /%cmd% Release /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makeReleasePackage
echo build release configuration failed
exit 1
:makeReleasePackage
echo Make package Release
%devenv% macroscope.sln /%cmd% Release /Project PACKAGE.vcproj /Log
if errorlevel 0 goto makeRelWithDebInfo
echo make release package failed
exit 1

:makeRelWithDebInfo
echo Build configuration RelWithDebInfo %x64%
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo %x64%
if errorlevel 0 goto buildRelWithDebInfo
echo cmake execution failed
exit 1
:buildRelWithDebInfo
%devenv% macroscope.sln /%cmd% RelWithDebInfo /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makeRelWithDebInfoPackage
echo build RelWithDebInfo configuration failed
exit 1
:makeRelWithDebInfoPackage
echo Make package RelWithDebInfo
%devenv% macroscope.sln /%cmd% RelWithDebInfo /Project PACKAGE.vcproj /Log
if errorlevel 0 goto makeMinSizeRel
echo make RelWithDebInfoPackage package failed
exit 1

:makeMinSizeRel
echo Build configuration MinSizeRel %x64%
cmake -DCMAKE_BUILD_TYPE=MinSizeRel %x64%
if errorlevel 0 goto buildMinSizeRel
echo cmake execution failed
exit 1
:buildMinSizeRel
%devenv% macroscope.sln /%cmd% MinSizeRel /Project ALL_BUILD.vcproj /Log
if errorlevel 0 goto makeMinSizeRelPackage
echo build MinSizeRel configuration failed
exit 1
:makeMinSizeRelPackage
echo Make package MinSizeRel
%devenv% macroscope.sln /%cmd% MinSizeRel /Project PACKAGE.vcproj /Log
if errorlevel 0 goto success
echo make MinSizeRel package failed
exit 1

:success
