// wrapper for dlldata.c

#ifdef _MERGE_PROXYSTUB // merge proxy stub DLL

#define REGISTER_PROXY_DLL //DllRegisterServer, etc.

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500	//for WinNT 4.0 or Win95 with DCOM
#endif
#define USE_STUBLESS_PROXY	//defined only with MIDL switch /Oicf

#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define ENTRY_PREFIX	Prx

#ifndef CMAKE_BUILD
#include "dlldata.c"
#include "msmail1clib_p.c"
#elif CMAKE_BUILD_TYPE == 1
#include "msmail1clib.dir/Debug/dlldata.c"
#include "msmail1clib.dir/Debug/msmail1clib_i.c"
#include "msmail1clib.dir/Debug/msmail1clib_p.c"
#elif CMAKE_BUILD_TYPE == 2
#include "msmail1clib.dir/Release/dlldata.c"
#include "msmail1clib.dir/Release/msmail1clib_i.c"
#include "msmail1clib.dir/Release/msmail1clib_p.c"
#elif CMAKE_BUILD_TYPE == 3
#include "msmail1clib.dir/RelWithDebInf/dlldata.c"
#include "msmail1clib.dir/RelWithDebInf/msmail1clib_i.c"
#include "msmail1clib.dir/RelWithDebInf/msmail1clib_p.c"
#elif CMAKE_BUILD_TYPE == 4
#include "msmail1clib.dir/MinSizeRel/dlldata.c"
#include "msmail1clib.dir/MinSizeRel/msmail1clib_i.c"
#include "msmail1clib.dir/MinSizeRel/msmail1clib_p.c"
#endif

#endif //_MERGE_PROXYSTUB
