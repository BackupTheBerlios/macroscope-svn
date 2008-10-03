@echo off

if NOT DEFINED INTEL_LICENSE_FILE goto exit

set cmake_defs=-DCMAKE_C_COMPILER=icl -DCMAKE_CXX_COMPILER=icl -DCMAKE_LINKER=xilink

if "%1" == "" goto x86
if /i %1 == x86       goto x86
if /i %1 == amd64     goto amd64
if /i %1 == x64       goto amd64
if /i %1 == ia64      goto ia64

:x86
rem call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\10.1.021\IA32\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit2
:amd64
rem call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\10.1.021\EM64T\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit2
:ia64
rem call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\10.1.021\Itanium\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit2

:exit


if DEFINED VS90COMNTOOLS (
  call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" %1 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
)
if DEFINED VS80COMNTOOLS (
  call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" %1 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
)
if DEFINED VS71COMNTOOLS (
  call "%VS71COMNTOOLS%..\..\VC\vcvarsall.bat" %1 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
)
:exit2
rem del /s /f /q *.obj *.exe *.dll *.ilk *.upx 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
cmake -DPRIVATE_RELEASE=1 -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCUMENTATION=ON -DDOCUMENTATION_HTML_TARZ=ON -G "NMake Makefiles" %cmake_defs% . && nmake /nologo
