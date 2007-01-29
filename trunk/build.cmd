@echo off

call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat"

cmake -G "NMake Makefiles" .
