@echo off

call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat"

midl /D "_CRT_SECURE_NO_DEPRECATE" /D "HAVE_CONFIG_H" /D "CMAKE_BUILD" /D "CMAKE_SYSTEM_NAME="Windows"" /D "CMAKE_BUILD_TYPE=2" /D "_WIN32_WINNT=0x0502" /D "DISABLE_FIREBIRD_INTERFACE" /D "DISABLE_MYSQL_INTERFACE" /D "_USRDLL" /D "_MERGE_PROXYSTUB" /nologo /char signed /env win32 /Oicf  /tlb "msmail1clib.tlb" /out "msmail1clib.dir\Release" /h "msmail1clib.h" /iid "msmail1clib_i.c" /proxy "msmail1clib_p.c" .\msmail1clib.idl