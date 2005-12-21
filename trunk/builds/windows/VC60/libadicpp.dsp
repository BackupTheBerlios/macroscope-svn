# Microsoft Developer Studio Project File - Name="libadicpp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libadicpp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libadicpp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libadicpp.mak" CFG="libadicpp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libadicpp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libadicpp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libadicppRelease"
# PROP Intermediate_Dir "libadicppRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /Ob2 /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINNT=0x400 /D "EMBED_ALL_CP" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\binaries\libadicpp_ms.lib"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libadicppDebug"
# PROP Intermediate_Dir "libadicppDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_WINNT=0x400 /D "EMBED_ALL_CP" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\binaries\libadicpp_debug_ms.lib"

!ENDIF 

# Begin Target

# Name "libadicpp - Win32 Release"
# Name "libadicpp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "libksys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libksys\conf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\except.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\fhc.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\log.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\mstream.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\mutant.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\mutex.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\profiler.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\rnd.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\sysutils.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\thread.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\xalloc.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libksys\xtime.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libksysRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libksysDebug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libutf8"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10000.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10001.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10002.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10003.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10004.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10005.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10006.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10007.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10008.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10010.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10017.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10021.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10029.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10079.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10081.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp10082.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1026.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1047.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1140.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1141.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1142.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1143.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1144.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1145.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1146.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1147.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1148.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1149.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1250.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1251.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1252.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1253.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1254.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1255.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1256.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1257.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1258.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp1361.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20000.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20001.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20002.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20003.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20004.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20005.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20105.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20106.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20107.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20108.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20127.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20261.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20269.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20273.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20277.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20278.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20280.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20284.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20285.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20290.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20297.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20420.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20423.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20424.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20833.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20838.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20866.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20871.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20880.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20905.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20924.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20932.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20936.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp20949.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp21025.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp21027.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp21866.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28591.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28592.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28593.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28594.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28595.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28596.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28597.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28598.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28599.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp28605.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp37.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp38598.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp437.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp500.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50220.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50221.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50222.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50225.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50227.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp50229.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp51949.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp52936.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57002.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57003.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57004.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57005.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57006.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57007.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57008.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57009.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57010.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp57011.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp708.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp720.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp737.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp775.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp850.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp852.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp855.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp857.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp858.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp860.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp861.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp862.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp863.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp864.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp865.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp866.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp869.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp870.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp874.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp875.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp932.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp936.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp949.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\cp950.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8c1.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8c2.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8c3.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8cp.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8dtcp.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8func.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8lct.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8str.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutf8\utf8uct.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libutf8Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libutf8Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libfbcpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\db.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\ecs.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\event.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\fblibic.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\intf.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\st.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\stparam.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\stvalue.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\svc.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libfbcpp\tr.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libfbcppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libfbcppDebug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libmycpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libmycpp\db.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\ecs.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\intf.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\mylibic.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\st.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\stparam.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\stvalue.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libmycpp\tr.cpp

!IF  "$(CFG)" == "libadicpp - Win32 Release"

# PROP Intermediate_Dir "libmycppRelease"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libmycppDebug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libadicpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libadicpp\adilibic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libadicpp\db.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libadicpp\fb.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libadicpp\my.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libadicpp\st.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\adicpp\adicpp.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\array.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\autoptr.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\except.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbcpp.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbdb.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbevent.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbexcept.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbintf.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbst.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbsvc.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fbtr.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\fhc.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\hol.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\lconfig.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\list.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\log.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mstream.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mutant.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mutex.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mycpp.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mydb.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\myexcept.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\myintf.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\myst.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\mytr.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\profiler.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\qsort.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\rnd.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\sp.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\sysutils.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\thread.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\tlv.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\tree.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\utf8embd.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\utf8str.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\vector.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\xalloc.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\include\adicpp\xtime.h

!IF  "$(CFG)" == "libadicpp - Win32 Release"

!ELSEIF  "$(CFG)" == "libadicpp - Win32 Debug"

# PROP Intermediate_Dir "libadicppDebug"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
