@echo off

if "%1" == "" goto x86
if /i %1 == x86       goto x86
if /i %1 == amd64     goto amd64
if /i %1 == x64       goto amd64
if /i %1 == ia64      goto ia64

:x86
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\IA32\Bin\iclvars.bat"
goto exit
:amd64
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\EM64T\Bin\iclvars.bat"
goto exit
:ia64
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\Itanium\Bin\iclvars.bat"
goto exit

:exit

call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat"

cmake -DPRIVATE_RELEASE=1 -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" . &&
rem del /s /f /q *.obj *.exe *.dll 1>null 2>null
nmake && make package
