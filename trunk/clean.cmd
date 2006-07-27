@echo off
del /s /f /q *.o 2>nul
del /s /f /q *.a 2>nul
del /s /f /q *.la 2>nul
del /s /f /q *.la.lnk 2>nul
del /s /f /q *.lai 2>nul
del /s /f /q *.po 2>nul
del /s /f /q *.plo 2>nul
del /s /f /q *.obj 2>nul
del /s /f /q *.lo* 2>nul
del /s /f /q .lib 2>nul
del /s /f /q .libs 2>nul
del /s /f /q .deps 2>nul
del /f /q include\adicpp\mysql.h 2>nul
del /s /f /q mysql1.h mysql2.h 2>nul
del /s /f /q firebird.h 2>nul
del /s /f /q Makefile 2>nul
del /s /f /q Makefile.in 2>nul
del /s /f /q tools 2>nul
del /s /f /q autom4te.cache 2>nul
del /s /f /q configure 2>nul
del /s /f /q aclocal.m4 2>nul
del /s /f /q libtool 2>nul
del /s /f /q config.h 2>nul
del /s /f /q configure.lineno 2>nul
del /s /f /q config.h.in* 2>nul
del /s /f /q config.status 2>nul
del /s /f /q config.log 2>nul
del /s /f /q config.cache 2>nul
del /s /f /q *.exe 2>nul
del /s /f /q *.lib 2>nul
del /s /f /q *.cgl 2>nul
del /s /f /q *.csm 2>nul
del /s /f /q *.#* 2>nul
del /s /f /q *.drc 2>nul
del /s /f /q *.tds 2>nul
del /s /f /q *.idb 2>nul
del /s /f /q *.pch 2>nul
del /s /f /q *.pchi 2>nul
del /s /f /q *.pdb 2>nul
del /s /f /q *.sbr 2>nul
del /s /f /q *.ilk 2>nul
del /s /f /q *.plg 2>nul
del /s /f /q *.ncb 2>nul
del /s /f /q *.opt 2>nul
del /s /f /q *.htm 2>nul
del /s /f /q *.html 2>nul
del /s /f /q *.dg 2>nul
del /s /f /q *.dep 2>nul
del /s /f /q *.manifest 2>nul
del /s /f /q *.manifest.res 2>nul
del /s /f /q *.vcproj.*.user 2>nul
del /s /f /q *.log 2>nul
rd builds\windows\VC80\Win32\Debug_genutf8 2>nul
rd builds\windows\VC80\Win32\Release_genutf8 2>nul
rd builds\windows\VC80\Win32\Debug_libadicpp 2>nul
rd builds\windows\VC80\Win32\Release_libadicpp 2>nul
rd builds\windows\VC80\Win32\Debug_macroscope 2>nul
rd builds\windows\VC80\Win32\Release_macroscope 2>nul
rd builds\windows\VC80\Win32\Debug_msftp 2>nul
rd builds\windows\VC80\Win32\Release_msftp 2>nul
rd builds\windows\VC80\Win32\Debug_msftpd 2>nul
rd builds\windows\VC80\Win32\Release_msftpd 2>nul
rd builds\windows\VC80\Win32\Debug_msmail 2>nul
rd builds\windows\VC80\Win32\Release_msmail 2>nul
del /s /f /q builds\windows\VC80\Win32\Debug_msmail1clib\* 2>nul
rd builds\windows\VC80\Win32\Debug_msmail1clib 2>nul
del /s /f /q builds\windows\VC80\Win32\Release_msmail1clib\* 2>nul
rd builds\windows\VC80\Win32\Release_msmail1clib 2>nul
rd builds\windows\VC80\Win32 2>nul
rd builds\windows\VC80\x64\Debug_genutf8 2>nul
rd builds\windows\VC80\x64\Release_genutf8 2>nul
rd builds\windows\VC80\x64\Debug_libadicpp 2>nul
rd builds\windows\VC80\x64\Release_libadicpp 2>nul
rd builds\windows\VC80\x64\Debug_macroscope 2>nul
rd builds\windows\VC80\x64\Release_macroscope 2>nul
rd builds\windows\VC80\x64\Debug_msftp 2>nul
rd builds\windows\VC80\x64\Release_msftp 2>nul
rd builds\windows\VC80\x64\Debug_msftpd 2>nul
rd builds\windows\VC80\x64\Release_msftpd 2>nul
rd builds\windows\VC80\x64\Debug_msmail 2>nul
rd builds\windows\VC80\x64\Release_msmail 2>nul
rd builds\windows\VC80\x64 2>nul
rd "test\macroscope\html report" 2>nul
rd autom4te.cache 2>nul
rd tools 2>nul

del /s /f /q CMakeCache.txt install_manifest.txt cmake_install.cmake 2>nul
del /s /f /q CPack* 2>nul
del /s /f /q *.vcproj.cmake 2>nul
del /s /f /q INSTALL.vcproj PACKAGE.vcproj 2>nul

del /s /f /q builds\cmake\*.vcproj 2>nul
del /s /f /q builds\cmake\*.sln 2>nul
del /s /f /q /a:H builds\cmake\*.suo 2>nul
rd builds\cmake\ALL_BUILD.dir\Debug 2>nul
rd builds\cmake\ALL_BUILD.dir\Release 2>nul
rd builds\cmake\ALL_BUILD.dir\RelWithDebInf 2>nul
rd builds\cmake\ALL_BUILD.dir\MinSizeRel 2>nul
rd builds\cmake\ALL_BUILD.dir 2>nul
rd builds\cmake\PACKAGE.dir\Debug 2>nul
rd builds\cmake\PACKAGE.dir\Release 2>nul
rd builds\cmake\PACKAGE.dir\RelWithDebInf 2>nul
rd builds\cmake\PACKAGE.dir\MinSizeRel 2>nul
rd builds\cmake\PACKAGE.dir 2>nul
rd builds\cmake\ZERO_CHECK.dir\Debug 2>nul
rd builds\cmake\ZERO_CHECK.dir\Release 2>nul
rd builds\cmake\ZERO_CHECK.dir\RelWithDebInf 2>nul
rd builds\cmake\ZERO_CHECK.dir\MinSizeRel 2>nul
rd builds\cmake\ZERO_CHECK.dir 2>nul
del /s /f /q builds\cmake\CMakeFiles\* 2>nul
rd builds\cmake\CMakeTmp 2>nul
rd builds\cmake\CMakeFiles 2>nul
del /s /f /q builds\cmake\_CPack_Packages\* 2>nul

del /s /f /q test\macroscope\*.vcproj 2>nul
del /s /f /q test\msftp\*.vcproj 2>nul
del /s /f /q test\msftpd\*.vcproj 2>nul
del /s /f /q test\msmail\*.vcproj 2>nul
del /s /f /q test\msmail1clib\*.vcproj 2>nul
