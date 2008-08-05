@echo off

call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x64

cl *.cpp -FeTaste.exe /Ox /GL
del *.obj
