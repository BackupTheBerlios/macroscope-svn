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
rd builds\windows\VC70\libadicppDebug  2>nul
rd builds\windows\VC70\libadicppRelease  2>nul
rd builds\windows\VC70\libksysDebug  2>nul
rd builds\windows\VC70\libksysRelease  2>nul
rd builds\windows\VC70\libfbcppDebug  2>nul
rd builds\windows\VC70\libfbcppRelease  2>nul
rd builds\windows\VC70\libmycppDebug  2>nul
rd builds\windows\VC70\libmycppRelease  2>nul
rd builds\windows\VC70\libutf8Debug  2>nul
rd builds\windows\VC70\libutf8Release  2>nul
rd test\macroscope\VC70\macroscopeDebug 2>nul
rd test\macroscope\VC70\macroscopeRelease 2>nul
rd "test\macroscope\html report" 2>nul
rd test\msftpd\VC70\Debug
rd test\msftpd\VC70\Release
rd test\msftp\VC70\Debug
rd test\msftp\VC70\Release
rd autom4te.cache 2>nul
rd tools 2>nul
