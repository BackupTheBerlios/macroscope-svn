/*-
 * Copyright 2005 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
Exception * Exception::newObject()
{
  ksys::AutoPtr<Exception> safe((Exception *) ksys::kmalloc(sizeof(Exception)));
  new (safe) Exception;
  return safe.ptr(NULL);
}
//---------------------------------------------------------------------------
Exception::~Exception()
{
}
//---------------------------------------------------------------------------
Exception::Exception() : refCount_(0), stackBackTrace_(true)
{
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const utf8::String & what) : refCount_(0), stackBackTrace_(true)
{
  codes_.add(code);
  whats_.add(what);
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const char * what) : refCount_(0), stackBackTrace_(true)
{
  codes_.add(code);
  whats_.add(what);
}
//---------------------------------------------------------------------------
bool Exception::isFatalError() const
{
  return false;
}
//---------------------------------------------------------------------------
void Exception::throwSP()
{
/*
// Set options 
	DWORD options = SymGetOptions(); 
// SYMOPT_DEBUG option asks DbgHelp to print additional troubleshooting 
// messages to debug output - use the debugger's Debug Output window 
// to view the messages 
	options |= SYMOPT_DEBUG | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_INCLUDE_32BIT_MODULES;
	SymSetOptions(options);

  int32_t err;
  BOOL r = SymInitialize(GetCurrentProcess(),NULL,TRUE);
  err = GetLastError();
//  DWORD64 modBase = SymLoadModule64(GetCurrentProcess(),NULL,NULL,NULL,0,0);
//  err = GetLastError();
  union {
    SYMBOL_INFO SymbolInfo;
    char name[sizeof(SYMBOL_INFO) / sizeof(char) + MAX_SYM_NAME];
  };
  SymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
  SymbolInfo.MaxNameLen = MAX_SYM_NAME;
  DWORD64 dw64Displacement;
  r = SymFromAddr(
    GetCurrentProcess(),
    (DWORD64) fibonacci,
    &dw64Displacement,
    &SymbolInfo
  );
  err = GetLastError();
  if( SymbolInfo.Flags & SYMFLAG_THUNK ){
    SymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
    SymbolInfo.MaxNameLen = MAX_SYM_NAME;
    r = SymFromAddr(
      GetCurrentProcess(),
      SymbolInfo.Value,
      &dw64Displacement,
      &SymbolInfo
    );
    err = GetLastError();
  }
  if( SymbolInfo.Flags & SYMFLAG_FUNCTION || SymbolInfo.Flags == 0 ){
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD dwDisplacement;
    r = SymGetLineFromAddr64(GetCurrentProcess(),SymbolInfo.Address,&dwDisplacement,&line);
    err = GetLastError();
    union {
      DWORD * pTag;
      DWORD callingConvention;
      BSTR symName;
    };
    r = SymGetTypeInfo(
      GetCurrentProcess(),
      SymbolInfo.ModBase,
      SymbolInfo.TypeIndex,
      TI_GET_TYPE,
      &callingConvention
    );
    err = GetLastError();
    r = SymGetTypeInfo(
      GetCurrentProcess(),
      SymbolInfo.ModBase,
      SymbolInfo.TypeIndex,
      TI_GET_SYMTAG,
      &pTag
    );
    err = GetLastError();
    r = SymGetTypeInfo(
      GetCurrentProcess(),
      SymbolInfo.ModBase,
      SymbolInfo.TypeIndex,
      TI_GET_CALLING_CONVENTION,
      &callingConvention
    );
    err = GetLastError();
    r = SymGetTypeInfo(
      GetCurrentProcess(),
      SymbolInfo.ModBase,
      SymbolInfo.TypeIndex,
      TI_GET_SYMNAME,
      &symName
    );
    err = GetLastError();
    LocalFree(symName);
  }
  r = SymCleanup(GetCurrentProcess());
  err = GetLastError();*/

  if( stackBackTrace_ ){
    utf8::String::Stream stackTrace;
    stackTrace << getBackTrace();
    if( stackTrace.count() > 0 ) stdErr.debug(128,stackTrace);
  }
  refCount_ = 0;
  throw ExceptionSP(this);
}
//---------------------------------------------------------------------------
const Exception & Exception::writeStdError(LogFile * log) const
{
  if( stdErr.isDebugLevelEnabled(9) ){
    if( log == NULL ) log = &stdErr;
    for( uintptr_t i = 0; i < whats_.count(); i++ ){
      if( codes_[0] == 0 ) continue;
      intmax_t a;
      utf8::String serr(strError(codes_[i]));
      utf8::String::Stream s;
      if( !utf8::tryStr2Int(serr,a) ){
        if( codes_[i] >= errorOffset ) s << codes_[i] - errorOffset; else s << codes_[i];
        s << " ";
      }
      if( serr.strlen() > 0 ) s << serr << " ";
      s << whats_[i] << "\n";
      log->debug(9,s);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

