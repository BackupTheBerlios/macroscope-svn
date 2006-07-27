

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Fri Jul 21 10:56:34 2006
 */
/* Compiler settings for .\msmail1clib.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IInitDone,0xAB634001,0xF13D,0x11d0,0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA);


MIDL_DEFINE_GUID(IID, IID_IPropertyProfile,0xAB634002,0xF13D,0x11d0,0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA);


MIDL_DEFINE_GUID(IID, IID_IAsyncEvent,0xab634004,0xf13d,0x11d0,0xa4,0x59,0x00,0x40,0x95,0xe1,0xda,0xea);


MIDL_DEFINE_GUID(IID, IID_ILanguageExtender,0xAB634003,0xF13D,0x11d0,0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA);


MIDL_DEFINE_GUID(IID, IID_IStatusLine,0xab634005,0xf13d,0x11d0,0xa4,0x59,0x00,0x40,0x95,0xe1,0xda,0xea);


MIDL_DEFINE_GUID(IID, IID_Imsmail1c,0x0D8A8085,0x71CA,0x4889,0x9E,0x24,0x0C,0x46,0x96,0x66,0x38,0x46);


MIDL_DEFINE_GUID(IID, LIBID_msmail1clibLib,0x4BC03D2E,0x0898,0x4A3A,0xAC,0xEC,0x33,0x5F,0xEF,0xFE,0x3D,0x24);


MIDL_DEFINE_GUID(CLSID, CLSID_msmail1c,0x2F1A8B1C,0xDF62,0x4A85,0xA7,0x82,0x8E,0x23,0x6F,0xE3,0x35,0xFA);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



