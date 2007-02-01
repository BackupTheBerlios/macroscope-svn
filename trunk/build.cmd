@echo off

if "%1" == "" goto x86
if /i %1 == x86       goto x86
if /i %1 == amd64     goto amd64
if /i %1 == x64       goto amd64
if /i %1 == ia64      goto ia64

:x86
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\IA32\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit
:amd64
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\EM64T\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit
:ia64
call "%INTEL_LICENSE_FILE%\..\..\..\Intel\Compiler\C++\9.1\Itanium\Bin\iclvars.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
goto exit

:exit

call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log

rem del /s /f /q *.obj *.exe *.dll *.ilk *.upx 1>%TEMP%\stdout.log 2>%TEMP%\stderr.log
cmake -DPRIVATE_RELEASE=1 -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" . && nmake && make package
