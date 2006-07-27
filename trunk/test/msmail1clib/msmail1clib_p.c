

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */
#pragma warning( disable: 4211 )  /* redefine extent to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "msmail1clib.h"

#define TYPE_FORMAT_STRING_SIZE   1111                              
#define PROC_FORMAT_STRING_SIZE   803                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   3            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IInitDone_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IInitDone_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPropertyProfile_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPropertyProfile_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IAsyncEvent_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAsyncEvent_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ILanguageExtender_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ILanguageExtender_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IStatusLine_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IStatusLine_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO Imsmail1c_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO Imsmail1c_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure Init */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x3 ),	/* 3 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter pConnection */

/* 16 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 18 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 20 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */

	/* Return value */

/* 22 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 24 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 26 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ResetStatusLine */


	/* Procedure Done */

/* 28 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 30 */	NdrFcLong( 0x0 ),	/* 0 */
/* 34 */	NdrFcShort( 0x4 ),	/* 4 */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	NdrFcShort( 0x0 ),	/* 0 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	0x4,		/* Oi2 Flags:  has return, */
			0x1,		/* 1 */

	/* Return value */


	/* Return value */

/* 44 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 46 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 48 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetInfo */

/* 50 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 52 */	NdrFcLong( 0x0 ),	/* 0 */
/* 56 */	NdrFcShort( 0x5 ),	/* 5 */
/* 58 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 60 */	NdrFcShort( 0x0 ),	/* 0 */
/* 62 */	NdrFcShort( 0x8 ),	/* 8 */
/* 64 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter pInfo */

/* 66 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 68 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 70 */	NdrFcShort( 0x3f2 ),	/* Type Offset=1010 */

	/* Return value */

/* 72 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 74 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 76 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RegisterProfileAs */

/* 78 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 80 */	NdrFcLong( 0x0 ),	/* 0 */
/* 84 */	NdrFcShort( 0x5 ),	/* 5 */
/* 86 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 88 */	NdrFcShort( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x8 ),	/* 8 */
/* 92 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter bstrProfileName */

/* 94 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 96 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 98 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Return value */

/* 100 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 102 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 104 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure SetEventBufferDepth */

/* 106 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 108 */	NdrFcLong( 0x0 ),	/* 0 */
/* 112 */	NdrFcShort( 0x3 ),	/* 3 */
/* 114 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 116 */	NdrFcShort( 0x8 ),	/* 8 */
/* 118 */	NdrFcShort( 0x8 ),	/* 8 */
/* 120 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter lDepth */

/* 122 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 124 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 126 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 128 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 130 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 132 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetEventBufferDepth */

/* 134 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 136 */	NdrFcLong( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x4 ),	/* 4 */
/* 142 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 144 */	NdrFcShort( 0x1c ),	/* 28 */
/* 146 */	NdrFcShort( 0x8 ),	/* 8 */
/* 148 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter plDepth */

/* 150 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 152 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 154 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 156 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 158 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 160 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ExternalEvent */

/* 162 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 164 */	NdrFcLong( 0x0 ),	/* 0 */
/* 168 */	NdrFcShort( 0x5 ),	/* 5 */
/* 170 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 172 */	NdrFcShort( 0x0 ),	/* 0 */
/* 174 */	NdrFcShort( 0x8 ),	/* 8 */
/* 176 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x4,		/* 4 */

	/* Parameter bstrSource */

/* 178 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 180 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 182 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Parameter bstrMessage */

/* 184 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 186 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 188 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Parameter bstrData */

/* 190 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 192 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 194 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Return value */

/* 196 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 198 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 200 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure CleanBuffer */

/* 202 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 204 */	NdrFcLong( 0x0 ),	/* 0 */
/* 208 */	NdrFcShort( 0x6 ),	/* 6 */
/* 210 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x8 ),	/* 8 */
/* 216 */	0x4,		/* Oi2 Flags:  has return, */
			0x1,		/* 1 */

	/* Return value */

/* 218 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 220 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 222 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RegisterExtensionAs */

/* 224 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 226 */	NdrFcLong( 0x0 ),	/* 0 */
/* 230 */	NdrFcShort( 0x3 ),	/* 3 */
/* 232 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 234 */	NdrFcShort( 0x0 ),	/* 0 */
/* 236 */	NdrFcShort( 0x8 ),	/* 8 */
/* 238 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter bstrExtensionName */

/* 240 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 242 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 244 */	NdrFcShort( 0x412 ),	/* Type Offset=1042 */

	/* Return value */

/* 246 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 248 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 250 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetNProps */

/* 252 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 254 */	NdrFcLong( 0x0 ),	/* 0 */
/* 258 */	NdrFcShort( 0x4 ),	/* 4 */
/* 260 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 262 */	NdrFcShort( 0x1c ),	/* 28 */
/* 264 */	NdrFcShort( 0x24 ),	/* 36 */
/* 266 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter plProps */

/* 268 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 270 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 272 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 274 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 276 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 278 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure FindProp */

/* 280 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 282 */	NdrFcLong( 0x0 ),	/* 0 */
/* 286 */	NdrFcShort( 0x5 ),	/* 5 */
/* 288 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 290 */	NdrFcShort( 0x1c ),	/* 28 */
/* 292 */	NdrFcShort( 0x24 ),	/* 36 */
/* 294 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter bstrPropName */

/* 296 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 298 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 300 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Parameter plPropNum */

/* 302 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 304 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 306 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 308 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 310 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 312 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetPropName */

/* 314 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 316 */	NdrFcLong( 0x0 ),	/* 0 */
/* 320 */	NdrFcShort( 0x6 ),	/* 6 */
/* 322 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 324 */	NdrFcShort( 0x10 ),	/* 16 */
/* 326 */	NdrFcShort( 0x8 ),	/* 8 */
/* 328 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x4,		/* 4 */

	/* Parameter lPropNum */

/* 330 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 332 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 334 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter lPropAlias */

/* 336 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 338 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 340 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pbstrPropName */

/* 342 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 344 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 346 */	NdrFcShort( 0x412 ),	/* Type Offset=1042 */

	/* Return value */

/* 348 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 350 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 352 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetPropVal */

/* 354 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 356 */	NdrFcLong( 0x0 ),	/* 0 */
/* 360 */	NdrFcShort( 0x7 ),	/* 7 */
/* 362 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 364 */	NdrFcShort( 0x8 ),	/* 8 */
/* 366 */	NdrFcShort( 0x8 ),	/* 8 */
/* 368 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter lPropNum */

/* 370 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 372 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 374 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pvarPropVal */

/* 376 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 378 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 380 */	NdrFcShort( 0x424 ),	/* Type Offset=1060 */

	/* Return value */

/* 382 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 384 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 386 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure SetPropVal */

/* 388 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 390 */	NdrFcLong( 0x0 ),	/* 0 */
/* 394 */	NdrFcShort( 0x8 ),	/* 8 */
/* 396 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 398 */	NdrFcShort( 0x8 ),	/* 8 */
/* 400 */	NdrFcShort( 0x8 ),	/* 8 */
/* 402 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter lPropNum */

/* 404 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 406 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 408 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter varPropVal */

/* 410 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 412 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 414 */	NdrFcShort( 0x436 ),	/* Type Offset=1078 */

	/* Return value */

/* 416 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 418 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 420 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure IsPropReadable */

/* 422 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 424 */	NdrFcLong( 0x0 ),	/* 0 */
/* 428 */	NdrFcShort( 0x9 ),	/* 9 */
/* 430 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 432 */	NdrFcShort( 0x24 ),	/* 36 */
/* 434 */	NdrFcShort( 0x24 ),	/* 36 */
/* 436 */	0x4,		/* Oi2 Flags:  has return, */
			0x3,		/* 3 */

	/* Parameter lPropNum */

/* 438 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 440 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 442 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pboolPropRead */

/* 444 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 446 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 448 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 450 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 452 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 454 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure IsPropWritable */

/* 456 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 458 */	NdrFcLong( 0x0 ),	/* 0 */
/* 462 */	NdrFcShort( 0xa ),	/* 10 */
/* 464 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 466 */	NdrFcShort( 0x24 ),	/* 36 */
/* 468 */	NdrFcShort( 0x24 ),	/* 36 */
/* 470 */	0x4,		/* Oi2 Flags:  has return, */
			0x3,		/* 3 */

	/* Parameter lPropNum */

/* 472 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 474 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 476 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pboolPropWrite */

/* 478 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 480 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 482 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 484 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 486 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 488 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetNMethods */

/* 490 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 492 */	NdrFcLong( 0x0 ),	/* 0 */
/* 496 */	NdrFcShort( 0xb ),	/* 11 */
/* 498 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 500 */	NdrFcShort( 0x1c ),	/* 28 */
/* 502 */	NdrFcShort( 0x24 ),	/* 36 */
/* 504 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter plMethods */

/* 506 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 508 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 510 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 512 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 514 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 516 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure FindMethod */

/* 518 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 520 */	NdrFcLong( 0x0 ),	/* 0 */
/* 524 */	NdrFcShort( 0xc ),	/* 12 */
/* 526 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 528 */	NdrFcShort( 0x1c ),	/* 28 */
/* 530 */	NdrFcShort( 0x24 ),	/* 36 */
/* 532 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter bstrMethodName */

/* 534 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 536 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 538 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Parameter plMethodNum */

/* 540 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 542 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 544 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 546 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 548 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 550 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetMethodName */

/* 552 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 554 */	NdrFcLong( 0x0 ),	/* 0 */
/* 558 */	NdrFcShort( 0xd ),	/* 13 */
/* 560 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 562 */	NdrFcShort( 0x10 ),	/* 16 */
/* 564 */	NdrFcShort( 0x8 ),	/* 8 */
/* 566 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x4,		/* 4 */

	/* Parameter lMethodNum */

/* 568 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 570 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 572 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter lMethodAlias */

/* 574 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 576 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 578 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pbstrMethodName */

/* 580 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 582 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 584 */	NdrFcShort( 0x412 ),	/* Type Offset=1042 */

	/* Return value */

/* 586 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 588 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 590 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetNParams */

/* 592 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 594 */	NdrFcLong( 0x0 ),	/* 0 */
/* 598 */	NdrFcShort( 0xe ),	/* 14 */
/* 600 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 602 */	NdrFcShort( 0x24 ),	/* 36 */
/* 604 */	NdrFcShort( 0x24 ),	/* 36 */
/* 606 */	0x4,		/* Oi2 Flags:  has return, */
			0x3,		/* 3 */

	/* Parameter lMethodNum */

/* 608 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 610 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 612 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter plParams */

/* 614 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 616 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 618 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 620 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 622 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 624 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetParamDefValue */

/* 626 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 628 */	NdrFcLong( 0x0 ),	/* 0 */
/* 632 */	NdrFcShort( 0xf ),	/* 15 */
/* 634 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 636 */	NdrFcShort( 0x10 ),	/* 16 */
/* 638 */	NdrFcShort( 0x8 ),	/* 8 */
/* 640 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x4,		/* 4 */

	/* Parameter lMethodNum */

/* 642 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 644 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 646 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter lParamNum */

/* 648 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 650 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 652 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pvarParamDefValue */

/* 654 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 656 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 658 */	NdrFcShort( 0x424 ),	/* Type Offset=1060 */

	/* Return value */

/* 660 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 662 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 664 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure HasRetVal */

/* 666 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 668 */	NdrFcLong( 0x0 ),	/* 0 */
/* 672 */	NdrFcShort( 0x10 ),	/* 16 */
/* 674 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 676 */	NdrFcShort( 0x24 ),	/* 36 */
/* 678 */	NdrFcShort( 0x24 ),	/* 36 */
/* 680 */	0x4,		/* Oi2 Flags:  has return, */
			0x3,		/* 3 */

	/* Parameter lMethodNum */

/* 682 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 684 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 686 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pboolRetValue */

/* 688 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 690 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 692 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 694 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 696 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 698 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure CallAsProc */

/* 700 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 702 */	NdrFcLong( 0x0 ),	/* 0 */
/* 706 */	NdrFcShort( 0x11 ),	/* 17 */
/* 708 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 710 */	NdrFcShort( 0x8 ),	/* 8 */
/* 712 */	NdrFcShort( 0x8 ),	/* 8 */
/* 714 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter lMethodNum */

/* 716 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 718 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 720 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter paParams */

/* 722 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 724 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 726 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */

	/* Return value */

/* 728 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 730 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 732 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure CallAsFunc */

/* 734 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 736 */	NdrFcLong( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x12 ),	/* 18 */
/* 742 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 744 */	NdrFcShort( 0x8 ),	/* 8 */
/* 746 */	NdrFcShort( 0x8 ),	/* 8 */
/* 748 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x4,		/* 4 */

	/* Parameter lMethodNum */

/* 750 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 752 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 754 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pvarRetValue */

/* 756 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 758 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 760 */	NdrFcShort( 0x424 ),	/* Type Offset=1060 */

	/* Parameter paParams */

/* 762 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 764 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 766 */	NdrFcShort( 0x44c ),	/* Type Offset=1100 */

	/* Return value */

/* 768 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 770 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 772 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure SetStatusLine */

/* 774 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 776 */	NdrFcLong( 0x0 ),	/* 0 */
/* 780 */	NdrFcShort( 0x3 ),	/* 3 */
/* 782 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0x8 ),	/* 8 */
/* 788 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter bstrStatusLine */

/* 790 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 792 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 794 */	NdrFcShort( 0x400 ),	/* Type Offset=1024 */

	/* Return value */

/* 796 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 798 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 800 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/*  4 */	NdrFcLong( 0x20400 ),	/* 132096 */
/*  8 */	NdrFcShort( 0x0 ),	/* 0 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 14 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 16 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 18 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 20 */	
			0x11, 0x0,	/* FC_RP */
/* 22 */	NdrFcShort( 0x3dc ),	/* Offset= 988 (1010) */
/* 24 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 26 */	NdrFcShort( 0x2 ),	/* Offset= 2 (28) */
/* 28 */	
			0x13, 0x0,	/* FC_OP */
/* 30 */	NdrFcShort( 0x3c2 ),	/* Offset= 962 (992) */
/* 32 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x49,		/* 73 */
/* 34 */	NdrFcShort( 0x18 ),	/* 24 */
/* 36 */	NdrFcShort( 0xa ),	/* 10 */
/* 38 */	NdrFcLong( 0x8 ),	/* 8 */
/* 42 */	NdrFcShort( 0x6c ),	/* Offset= 108 (150) */
/* 44 */	NdrFcLong( 0xd ),	/* 13 */
/* 48 */	NdrFcShort( 0x9e ),	/* Offset= 158 (206) */
/* 50 */	NdrFcLong( 0x9 ),	/* 9 */
/* 54 */	NdrFcShort( 0xba ),	/* Offset= 186 (240) */
/* 56 */	NdrFcLong( 0xc ),	/* 12 */
/* 60 */	NdrFcShort( 0x2b2 ),	/* Offset= 690 (750) */
/* 62 */	NdrFcLong( 0x24 ),	/* 36 */
/* 66 */	NdrFcShort( 0x2da ),	/* Offset= 730 (796) */
/* 68 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 72 */	NdrFcShort( 0x2f6 ),	/* Offset= 758 (830) */
/* 74 */	NdrFcLong( 0x10 ),	/* 16 */
/* 78 */	NdrFcShort( 0x30e ),	/* Offset= 782 (860) */
/* 80 */	NdrFcLong( 0x2 ),	/* 2 */
/* 84 */	NdrFcShort( 0x326 ),	/* Offset= 806 (890) */
/* 86 */	NdrFcLong( 0x3 ),	/* 3 */
/* 90 */	NdrFcShort( 0x33e ),	/* Offset= 830 (920) */
/* 92 */	NdrFcLong( 0x14 ),	/* 20 */
/* 96 */	NdrFcShort( 0x356 ),	/* Offset= 854 (950) */
/* 98 */	NdrFcShort( 0xffff ),	/* Offset= -1 (97) */
/* 100 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 102 */	NdrFcShort( 0x2 ),	/* 2 */
/* 104 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 106 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 108 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 110 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 112 */	NdrFcShort( 0x8 ),	/* 8 */
/* 114 */	NdrFcShort( 0xfff2 ),	/* Offset= -14 (100) */
/* 116 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 118 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 120 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 122 */	NdrFcShort( 0x4 ),	/* 4 */
/* 124 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 126 */	NdrFcShort( 0x0 ),	/* 0 */
/* 128 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 130 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x1 ),	/* 1 */
/* 138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x0 ),	/* 0 */
/* 142 */	0x13, 0x0,	/* FC_OP */
/* 144 */	NdrFcShort( 0xffde ),	/* Offset= -34 (110) */
/* 146 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 148 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 150 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 152 */	NdrFcShort( 0x8 ),	/* 8 */
/* 154 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 156 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 158 */	NdrFcShort( 0x4 ),	/* 4 */
/* 160 */	NdrFcShort( 0x4 ),	/* 4 */
/* 162 */	0x11, 0x0,	/* FC_RP */
/* 164 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (120) */
/* 166 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 168 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 170 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 172 */	NdrFcLong( 0x0 ),	/* 0 */
/* 176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 178 */	NdrFcShort( 0x0 ),	/* 0 */
/* 180 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 182 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 184 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 186 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 188 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 192 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 194 */	NdrFcShort( 0x0 ),	/* 0 */
/* 196 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 200 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 202 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (170) */
/* 204 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 206 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 208 */	NdrFcShort( 0x8 ),	/* 8 */
/* 210 */	NdrFcShort( 0x0 ),	/* 0 */
/* 212 */	NdrFcShort( 0x6 ),	/* Offset= 6 (218) */
/* 214 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 216 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 218 */	
			0x11, 0x0,	/* FC_RP */
/* 220 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (188) */
/* 222 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 226 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 228 */	NdrFcShort( 0x0 ),	/* 0 */
/* 230 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 234 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 236 */	NdrFcShort( 0xff16 ),	/* Offset= -234 (2) */
/* 238 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 240 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 244 */	NdrFcShort( 0x0 ),	/* 0 */
/* 246 */	NdrFcShort( 0x6 ),	/* Offset= 6 (252) */
/* 248 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 250 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 252 */	
			0x11, 0x0,	/* FC_RP */
/* 254 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (222) */
/* 256 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 258 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 260 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 262 */	NdrFcShort( 0x2 ),	/* Offset= 2 (264) */
/* 264 */	NdrFcShort( 0x10 ),	/* 16 */
/* 266 */	NdrFcShort( 0x2f ),	/* 47 */
/* 268 */	NdrFcLong( 0x14 ),	/* 20 */
/* 272 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 274 */	NdrFcLong( 0x3 ),	/* 3 */
/* 278 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 280 */	NdrFcLong( 0x11 ),	/* 17 */
/* 284 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 286 */	NdrFcLong( 0x2 ),	/* 2 */
/* 290 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 292 */	NdrFcLong( 0x4 ),	/* 4 */
/* 296 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 298 */	NdrFcLong( 0x5 ),	/* 5 */
/* 302 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 304 */	NdrFcLong( 0xb ),	/* 11 */
/* 308 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 310 */	NdrFcLong( 0xa ),	/* 10 */
/* 314 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 316 */	NdrFcLong( 0x6 ),	/* 6 */
/* 320 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (552) */
/* 322 */	NdrFcLong( 0x7 ),	/* 7 */
/* 326 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 328 */	NdrFcLong( 0x8 ),	/* 8 */
/* 332 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (558) */
/* 334 */	NdrFcLong( 0xd ),	/* 13 */
/* 338 */	NdrFcShort( 0xff58 ),	/* Offset= -168 (170) */
/* 340 */	NdrFcLong( 0x9 ),	/* 9 */
/* 344 */	NdrFcShort( 0xfeaa ),	/* Offset= -342 (2) */
/* 346 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 350 */	NdrFcShort( 0xd4 ),	/* Offset= 212 (562) */
/* 352 */	NdrFcLong( 0x24 ),	/* 36 */
/* 356 */	NdrFcShort( 0xd6 ),	/* Offset= 214 (570) */
/* 358 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 362 */	NdrFcShort( 0xd0 ),	/* Offset= 208 (570) */
/* 364 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 368 */	NdrFcShort( 0xfe ),	/* Offset= 254 (622) */
/* 370 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 374 */	NdrFcShort( 0xfc ),	/* Offset= 252 (626) */
/* 376 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 380 */	NdrFcShort( 0xfa ),	/* Offset= 250 (630) */
/* 382 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 386 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (634) */
/* 388 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 392 */	NdrFcShort( 0xf6 ),	/* Offset= 246 (638) */
/* 394 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 398 */	NdrFcShort( 0xf4 ),	/* Offset= 244 (642) */
/* 400 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 404 */	NdrFcShort( 0xde ),	/* Offset= 222 (626) */
/* 406 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 410 */	NdrFcShort( 0xdc ),	/* Offset= 220 (630) */
/* 412 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 416 */	NdrFcShort( 0xe6 ),	/* Offset= 230 (646) */
/* 418 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 422 */	NdrFcShort( 0xdc ),	/* Offset= 220 (642) */
/* 424 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 428 */	NdrFcShort( 0xde ),	/* Offset= 222 (650) */
/* 430 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 434 */	NdrFcShort( 0xdc ),	/* Offset= 220 (654) */
/* 436 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 440 */	NdrFcShort( 0xda ),	/* Offset= 218 (658) */
/* 442 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 446 */	NdrFcShort( 0xd8 ),	/* Offset= 216 (662) */
/* 448 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 452 */	NdrFcShort( 0xde ),	/* Offset= 222 (674) */
/* 454 */	NdrFcLong( 0x10 ),	/* 16 */
/* 458 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 460 */	NdrFcLong( 0x12 ),	/* 18 */
/* 464 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 466 */	NdrFcLong( 0x13 ),	/* 19 */
/* 470 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 472 */	NdrFcLong( 0x15 ),	/* 21 */
/* 476 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 478 */	NdrFcLong( 0x16 ),	/* 22 */
/* 482 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 484 */	NdrFcLong( 0x17 ),	/* 23 */
/* 488 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 490 */	NdrFcLong( 0xe ),	/* 14 */
/* 494 */	NdrFcShort( 0xbc ),	/* Offset= 188 (682) */
/* 496 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 500 */	NdrFcShort( 0xc0 ),	/* Offset= 192 (692) */
/* 502 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 506 */	NdrFcShort( 0xbe ),	/* Offset= 190 (696) */
/* 508 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 512 */	NdrFcShort( 0x72 ),	/* Offset= 114 (626) */
/* 514 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 518 */	NdrFcShort( 0x70 ),	/* Offset= 112 (630) */
/* 520 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 524 */	NdrFcShort( 0x6e ),	/* Offset= 110 (634) */
/* 526 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 530 */	NdrFcShort( 0x64 ),	/* Offset= 100 (630) */
/* 532 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 536 */	NdrFcShort( 0x5e ),	/* Offset= 94 (630) */
/* 538 */	NdrFcLong( 0x0 ),	/* 0 */
/* 542 */	NdrFcShort( 0x0 ),	/* Offset= 0 (542) */
/* 544 */	NdrFcLong( 0x1 ),	/* 1 */
/* 548 */	NdrFcShort( 0x0 ),	/* Offset= 0 (548) */
/* 550 */	NdrFcShort( 0xffff ),	/* Offset= -1 (549) */
/* 552 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 554 */	NdrFcShort( 0x8 ),	/* 8 */
/* 556 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 558 */	
			0x13, 0x0,	/* FC_OP */
/* 560 */	NdrFcShort( 0xfe3e ),	/* Offset= -450 (110) */
/* 562 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 564 */	NdrFcShort( 0x2 ),	/* Offset= 2 (566) */
/* 566 */	
			0x13, 0x0,	/* FC_OP */
/* 568 */	NdrFcShort( 0x1a8 ),	/* Offset= 424 (992) */
/* 570 */	
			0x13, 0x0,	/* FC_OP */
/* 572 */	NdrFcShort( 0x1e ),	/* Offset= 30 (602) */
/* 574 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 576 */	NdrFcLong( 0x2f ),	/* 47 */
/* 580 */	NdrFcShort( 0x0 ),	/* 0 */
/* 582 */	NdrFcShort( 0x0 ),	/* 0 */
/* 584 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 586 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 588 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 590 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 592 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 594 */	NdrFcShort( 0x1 ),	/* 1 */
/* 596 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 598 */	NdrFcShort( 0x4 ),	/* 4 */
/* 600 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 602 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 604 */	NdrFcShort( 0x10 ),	/* 16 */
/* 606 */	NdrFcShort( 0x0 ),	/* 0 */
/* 608 */	NdrFcShort( 0xa ),	/* Offset= 10 (618) */
/* 610 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 612 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 614 */	NdrFcShort( 0xffd8 ),	/* Offset= -40 (574) */
/* 616 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 618 */	
			0x13, 0x0,	/* FC_OP */
/* 620 */	NdrFcShort( 0xffe4 ),	/* Offset= -28 (592) */
/* 622 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 624 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 626 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 628 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 630 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 632 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 634 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 636 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 638 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 640 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 642 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 644 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 646 */	
			0x13, 0x0,	/* FC_OP */
/* 648 */	NdrFcShort( 0xffa0 ),	/* Offset= -96 (552) */
/* 650 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 652 */	NdrFcShort( 0xffa2 ),	/* Offset= -94 (558) */
/* 654 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 656 */	NdrFcShort( 0xfe1a ),	/* Offset= -486 (170) */
/* 658 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 660 */	NdrFcShort( 0xfd6e ),	/* Offset= -658 (2) */
/* 662 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 664 */	NdrFcShort( 0x2 ),	/* Offset= 2 (666) */
/* 666 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 668 */	NdrFcShort( 0x2 ),	/* Offset= 2 (670) */
/* 670 */	
			0x13, 0x0,	/* FC_OP */
/* 672 */	NdrFcShort( 0x140 ),	/* Offset= 320 (992) */
/* 674 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 676 */	NdrFcShort( 0x2 ),	/* Offset= 2 (678) */
/* 678 */	
			0x13, 0x0,	/* FC_OP */
/* 680 */	NdrFcShort( 0x14 ),	/* Offset= 20 (700) */
/* 682 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 684 */	NdrFcShort( 0x10 ),	/* 16 */
/* 686 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 688 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 690 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 692 */	
			0x13, 0x0,	/* FC_OP */
/* 694 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (682) */
/* 696 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 698 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 700 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 702 */	NdrFcShort( 0x20 ),	/* 32 */
/* 704 */	NdrFcShort( 0x0 ),	/* 0 */
/* 706 */	NdrFcShort( 0x0 ),	/* Offset= 0 (706) */
/* 708 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 710 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 712 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 714 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 716 */	NdrFcShort( 0xfe34 ),	/* Offset= -460 (256) */
/* 718 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 720 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 722 */	NdrFcShort( 0x4 ),	/* 4 */
/* 724 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 726 */	NdrFcShort( 0x0 ),	/* 0 */
/* 728 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 730 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 732 */	NdrFcShort( 0x4 ),	/* 4 */
/* 734 */	NdrFcShort( 0x0 ),	/* 0 */
/* 736 */	NdrFcShort( 0x1 ),	/* 1 */
/* 738 */	NdrFcShort( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x0 ),	/* 0 */
/* 742 */	0x13, 0x0,	/* FC_OP */
/* 744 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (700) */
/* 746 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 748 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 750 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 752 */	NdrFcShort( 0x8 ),	/* 8 */
/* 754 */	NdrFcShort( 0x0 ),	/* 0 */
/* 756 */	NdrFcShort( 0x6 ),	/* Offset= 6 (762) */
/* 758 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 760 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 762 */	
			0x11, 0x0,	/* FC_RP */
/* 764 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (720) */
/* 766 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 768 */	NdrFcShort( 0x4 ),	/* 4 */
/* 770 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 772 */	NdrFcShort( 0x0 ),	/* 0 */
/* 774 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 776 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 778 */	NdrFcShort( 0x4 ),	/* 4 */
/* 780 */	NdrFcShort( 0x0 ),	/* 0 */
/* 782 */	NdrFcShort( 0x1 ),	/* 1 */
/* 784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0x0 ),	/* 0 */
/* 788 */	0x13, 0x0,	/* FC_OP */
/* 790 */	NdrFcShort( 0xff44 ),	/* Offset= -188 (602) */
/* 792 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 794 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 796 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 798 */	NdrFcShort( 0x8 ),	/* 8 */
/* 800 */	NdrFcShort( 0x0 ),	/* 0 */
/* 802 */	NdrFcShort( 0x6 ),	/* Offset= 6 (808) */
/* 804 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 806 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 808 */	
			0x11, 0x0,	/* FC_RP */
/* 810 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (766) */
/* 812 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 814 */	NdrFcShort( 0x8 ),	/* 8 */
/* 816 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 818 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 820 */	NdrFcShort( 0x10 ),	/* 16 */
/* 822 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 824 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 826 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (812) */
			0x5b,		/* FC_END */
/* 830 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 832 */	NdrFcShort( 0x18 ),	/* 24 */
/* 834 */	NdrFcShort( 0x0 ),	/* 0 */
/* 836 */	NdrFcShort( 0xa ),	/* Offset= 10 (846) */
/* 838 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 840 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 842 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (818) */
/* 844 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 846 */	
			0x11, 0x0,	/* FC_RP */
/* 848 */	NdrFcShort( 0xfd6c ),	/* Offset= -660 (188) */
/* 850 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 852 */	NdrFcShort( 0x1 ),	/* 1 */
/* 854 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 856 */	NdrFcShort( 0x0 ),	/* 0 */
/* 858 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 860 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 862 */	NdrFcShort( 0x8 ),	/* 8 */
/* 864 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 866 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 868 */	NdrFcShort( 0x4 ),	/* 4 */
/* 870 */	NdrFcShort( 0x4 ),	/* 4 */
/* 872 */	0x13, 0x0,	/* FC_OP */
/* 874 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (850) */
/* 876 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 878 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 880 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 882 */	NdrFcShort( 0x2 ),	/* 2 */
/* 884 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 886 */	NdrFcShort( 0x0 ),	/* 0 */
/* 888 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 890 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 892 */	NdrFcShort( 0x8 ),	/* 8 */
/* 894 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 896 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 898 */	NdrFcShort( 0x4 ),	/* 4 */
/* 900 */	NdrFcShort( 0x4 ),	/* 4 */
/* 902 */	0x13, 0x0,	/* FC_OP */
/* 904 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (880) */
/* 906 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 908 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 910 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 912 */	NdrFcShort( 0x4 ),	/* 4 */
/* 914 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 916 */	NdrFcShort( 0x0 ),	/* 0 */
/* 918 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 920 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 922 */	NdrFcShort( 0x8 ),	/* 8 */
/* 924 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 926 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 928 */	NdrFcShort( 0x4 ),	/* 4 */
/* 930 */	NdrFcShort( 0x4 ),	/* 4 */
/* 932 */	0x13, 0x0,	/* FC_OP */
/* 934 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (910) */
/* 936 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 938 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 940 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 942 */	NdrFcShort( 0x8 ),	/* 8 */
/* 944 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 946 */	NdrFcShort( 0x0 ),	/* 0 */
/* 948 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 950 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 952 */	NdrFcShort( 0x8 ),	/* 8 */
/* 954 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 956 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 958 */	NdrFcShort( 0x4 ),	/* 4 */
/* 960 */	NdrFcShort( 0x4 ),	/* 4 */
/* 962 */	0x13, 0x0,	/* FC_OP */
/* 964 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (940) */
/* 966 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 968 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 970 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 972 */	NdrFcShort( 0x8 ),	/* 8 */
/* 974 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 976 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 978 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 980 */	NdrFcShort( 0x8 ),	/* 8 */
/* 982 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 984 */	NdrFcShort( 0xffd8 ),	/* -40 */
/* 986 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 988 */	NdrFcShort( 0xffee ),	/* Offset= -18 (970) */
/* 990 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 992 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 994 */	NdrFcShort( 0x28 ),	/* 40 */
/* 996 */	NdrFcShort( 0xffee ),	/* Offset= -18 (978) */
/* 998 */	NdrFcShort( 0x0 ),	/* Offset= 0 (998) */
/* 1000 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1002 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1004 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1006 */	NdrFcShort( 0xfc32 ),	/* Offset= -974 (32) */
/* 1008 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1010 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1012 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1014 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	NdrFcShort( 0xfc1e ),	/* Offset= -994 (24) */
/* 1020 */	
			0x12, 0x0,	/* FC_UP */
/* 1022 */	NdrFcShort( 0xfc70 ),	/* Offset= -912 (110) */
/* 1024 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1026 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1028 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1030 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1032 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1020) */
/* 1034 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1036 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 1038 */	
			0x11, 0x0,	/* FC_RP */
/* 1040 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1042) */
/* 1042 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1044 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1046 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1048 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1050 */	NdrFcShort( 0xfe14 ),	/* Offset= -492 (558) */
/* 1052 */	
			0x11, 0x0,	/* FC_RP */
/* 1054 */	NdrFcShort( 0x6 ),	/* Offset= 6 (1060) */
/* 1056 */	
			0x13, 0x0,	/* FC_OP */
/* 1058 */	NdrFcShort( 0xfe9a ),	/* Offset= -358 (700) */
/* 1060 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1062 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1064 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1066 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1068 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1056) */
/* 1070 */	
			0x11, 0x0,	/* FC_RP */
/* 1072 */	NdrFcShort( 0x6 ),	/* Offset= 6 (1078) */
/* 1074 */	
			0x12, 0x0,	/* FC_UP */
/* 1076 */	NdrFcShort( 0xfe88 ),	/* Offset= -376 (700) */
/* 1078 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1080 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1082 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1084 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1086 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1074) */
/* 1088 */	
			0x11, 0x0,	/* FC_RP */
/* 1090 */	NdrFcShort( 0xa ),	/* Offset= 10 (1100) */
/* 1092 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 1094 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1096) */
/* 1096 */	
			0x12, 0x0,	/* FC_UP */
/* 1098 */	NdrFcShort( 0xff96 ),	/* Offset= -106 (992) */
/* 1100 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1102 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1104 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1106 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1108 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (1092) */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            LPSAFEARRAY_UserSize
            ,LPSAFEARRAY_UserMarshal
            ,LPSAFEARRAY_UserUnmarshal
            ,LPSAFEARRAY_UserFree
            },
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            },
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IInitDone, ver. 0.0,
   GUID={0xAB634001,0xF13D,0x11d0,{0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA}} */

#pragma code_seg(".orpc")
static const unsigned short IInitDone_FormatStringOffsetTable[] =
    {
    0,
    28,
    50
    };

static const MIDL_STUBLESS_PROXY_INFO IInitDone_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IInitDone_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IInitDone_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IInitDone_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _IInitDoneProxyVtbl = 
{
    &IInitDone_ProxyInfo,
    &IID_IInitDone,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IInitDone::Init */ ,
    (void *) (INT_PTR) -1 /* IInitDone::Done */ ,
    (void *) (INT_PTR) -1 /* IInitDone::GetInfo */
};

const CInterfaceStubVtbl _IInitDoneStubVtbl =
{
    &IID_IInitDone,
    &IInitDone_ServerInfo,
    6,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPropertyBag, ver. 0.0,
   GUID={0x55272A00,0x42CB,0x11CE,{0x81,0x35,0x00,0xAA,0x00,0x4B,0xB8,0x51}} */


/* Object interface: IPropertyProfile, ver. 0.0,
   GUID={0xAB634002,0xF13D,0x11d0,{0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA}} */

#pragma code_seg(".orpc")
static const unsigned short IPropertyProfile_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    78
    };

static const MIDL_STUBLESS_PROXY_INFO IPropertyProfile_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IPropertyProfile_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPropertyProfile_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IPropertyProfile_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _IPropertyProfileProxyVtbl = 
{
    &IPropertyProfile_ProxyInfo,
    &IID_IPropertyProfile,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IPropertyBag_Read_Proxy */ ,
    0 /* (void *) (INT_PTR) -1 /* IPropertyBag::Write */ ,
    (void *) (INT_PTR) -1 /* IPropertyProfile::RegisterProfileAs */
};


static const PRPC_STUB_FUNCTION IPropertyProfile_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2
};

CInterfaceStubVtbl _IPropertyProfileStubVtbl =
{
    &IID_IPropertyProfile,
    &IPropertyProfile_ServerInfo,
    6,
    &IPropertyProfile_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IAsyncEvent, ver. 0.0,
   GUID={0xab634004,0xf13d,0x11d0,{0xa4,0x59,0x00,0x40,0x95,0xe1,0xda,0xea}} */

#pragma code_seg(".orpc")
static const unsigned short IAsyncEvent_FormatStringOffsetTable[] =
    {
    106,
    134,
    162,
    202
    };

static const MIDL_STUBLESS_PROXY_INFO IAsyncEvent_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IAsyncEvent_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IAsyncEvent_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IAsyncEvent_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _IAsyncEventProxyVtbl = 
{
    &IAsyncEvent_ProxyInfo,
    &IID_IAsyncEvent,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IAsyncEvent::SetEventBufferDepth */ ,
    (void *) (INT_PTR) -1 /* IAsyncEvent::GetEventBufferDepth */ ,
    (void *) (INT_PTR) -1 /* IAsyncEvent::ExternalEvent */ ,
    (void *) (INT_PTR) -1 /* IAsyncEvent::CleanBuffer */
};

const CInterfaceStubVtbl _IAsyncEventStubVtbl =
{
    &IID_IAsyncEvent,
    &IAsyncEvent_ServerInfo,
    7,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: ILanguageExtender, ver. 0.0,
   GUID={0xAB634003,0xF13D,0x11d0,{0xA4,0x59,0x00,0x40,0x95,0xE1,0xDA,0xEA}} */

#pragma code_seg(".orpc")
static const unsigned short ILanguageExtender_FormatStringOffsetTable[] =
    {
    224,
    252,
    280,
    314,
    354,
    388,
    422,
    456,
    490,
    518,
    552,
    592,
    626,
    666,
    700,
    734
    };

static const MIDL_STUBLESS_PROXY_INFO ILanguageExtender_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ILanguageExtender_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ILanguageExtender_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ILanguageExtender_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(19) _ILanguageExtenderProxyVtbl = 
{
    &ILanguageExtender_ProxyInfo,
    &IID_ILanguageExtender,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::RegisterExtensionAs */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetNProps */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::FindProp */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetPropName */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetPropVal */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::SetPropVal */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::IsPropReadable */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::IsPropWritable */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetNMethods */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::FindMethod */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetMethodName */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetNParams */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::GetParamDefValue */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::HasRetVal */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::CallAsProc */ ,
    (void *) (INT_PTR) -1 /* ILanguageExtender::CallAsFunc */
};

const CInterfaceStubVtbl _ILanguageExtenderStubVtbl =
{
    &IID_ILanguageExtender,
    &ILanguageExtender_ServerInfo,
    19,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IStatusLine, ver. 0.0,
   GUID={0xab634005,0xf13d,0x11d0,{0xa4,0x59,0x00,0x40,0x95,0xe1,0xda,0xea}} */

#pragma code_seg(".orpc")
static const unsigned short IStatusLine_FormatStringOffsetTable[] =
    {
    774,
    28
    };

static const MIDL_STUBLESS_PROXY_INFO IStatusLine_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IStatusLine_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IStatusLine_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IStatusLine_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _IStatusLineProxyVtbl = 
{
    &IStatusLine_ProxyInfo,
    &IID_IStatusLine,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IStatusLine::SetStatusLine */ ,
    (void *) (INT_PTR) -1 /* IStatusLine::ResetStatusLine */
};

const CInterfaceStubVtbl _IStatusLineStubVtbl =
{
    &IID_IStatusLine,
    &IStatusLine_ServerInfo,
    5,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: Imsmail1c, ver. 0.0,
   GUID={0x0D8A8085,0x71CA,0x4889,{0x9E,0x24,0x0C,0x46,0x96,0x66,0x38,0x46}} */

#pragma code_seg(".orpc")
static const unsigned short Imsmail1c_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO Imsmail1c_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &Imsmail1c_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO Imsmail1c_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &Imsmail1c_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _Imsmail1cProxyVtbl = 
{
    0,
    &IID_Imsmail1c,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


static const PRPC_STUB_FUNCTION Imsmail1c_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _Imsmail1cStubVtbl =
{
    &IID_Imsmail1c,
    &Imsmail1c_ServerInfo,
    7,
    &Imsmail1c_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x600016e, /* MIDL Version 6.0.366 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0   /* Reserved5 */
    };

const CInterfaceProxyVtbl * _msmail1clib_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IInitDoneProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPropertyProfileProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ILanguageExtenderProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IAsyncEventProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IStatusLineProxyVtbl,
    ( CInterfaceProxyVtbl *) &_Imsmail1cProxyVtbl,
    0
};

const CInterfaceStubVtbl * _msmail1clib_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IInitDoneStubVtbl,
    ( CInterfaceStubVtbl *) &_IPropertyProfileStubVtbl,
    ( CInterfaceStubVtbl *) &_ILanguageExtenderStubVtbl,
    ( CInterfaceStubVtbl *) &_IAsyncEventStubVtbl,
    ( CInterfaceStubVtbl *) &_IStatusLineStubVtbl,
    ( CInterfaceStubVtbl *) &_Imsmail1cStubVtbl,
    0
};

PCInterfaceName const _msmail1clib_InterfaceNamesList[] = 
{
    "IInitDone",
    "IPropertyProfile",
    "ILanguageExtender",
    "IAsyncEvent",
    "IStatusLine",
    "Imsmail1c",
    0
};

const IID *  _msmail1clib_BaseIIDList[] = 
{
    0,
    &IID_IPropertyBag,
    0,
    0,
    0,
    &IID_IDispatch,
    0
};


#define _msmail1clib_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _msmail1clib, pIID, n)

int __stdcall _msmail1clib_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _msmail1clib, 6, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _msmail1clib, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _msmail1clib, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _msmail1clib, 6, *pIndex )
    
}

const ExtendedProxyFileInfo msmail1clib_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _msmail1clib_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _msmail1clib_StubVtblList,
    (const PCInterfaceName * ) & _msmail1clib_InterfaceNamesList,
    (const IID ** ) & _msmail1clib_BaseIIDList,
    & _msmail1clib_IID_Lookup, 
    6,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

