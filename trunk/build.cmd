@echo off

call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat"

cmake -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" .
nmake
