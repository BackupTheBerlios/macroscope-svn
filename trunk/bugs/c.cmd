@echo off

cl -c -nologo -YX -MD -GR -GX -Od -D _WIN32_WINNT=0x0400 -D "WIN32" array.cpp
link /OPT:NOREF array.obj fbclient_ms.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

rem xicl6 -c -nologo -YX -MD -GR -GX -Od -D _WIN32_WINNT=0x0400 -D "WIN32" array.cpp
rem xilink6 /OPT:NOREF array.obj fbclient_ms.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

set path=%path%;c:\bcc\bin

rem bcc32 -Od -y -v -IC:\BCC\Include -LC:\BCC\Lib -D_WIN32_WINNT=0x0400 array.cpp fbclient_bor.lib