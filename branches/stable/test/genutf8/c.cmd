@echo off

xicl6 /nologo /MD /W3 /GR /GX /Ob2 /O3 /Qscalar_rep /Qunroll /Qprefetch /QaxKW /Qipo /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0400 /FD genutf8.cpp user32.lib