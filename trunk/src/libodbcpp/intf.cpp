/*-
 * Copyright 2007 Guram Dukashvili
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
#include <adicpp/odbcpp.h>
//---------------------------------------------------------------------------
using namespace ksys;
using namespace utf8;
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
API api;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
const char * const API::symbols_[] = {
  "SQLAllocHandle",
  "SQLSetEnvAttr",
  "SQLSetConnectAttr",
  "SQLConnect",
  "SQLDisconnect",
  "SQLFreeHandle",
  "SQLBindParam",
  "SQLEndTran",
  "SQLError",
  "SQLExecDirect",
  "SQLExecute",
  "SQLFetch"
};
//---------------------------------------------------------------------------
void API::initialize()
{
  new (mutex_) InterlockedMutex;
  new (clientLibrary_) utf8::String;
  count_ = 0;
}
//---------------------------------------------------------------------------
void API::cleanup()
{
  using namespace utf8;
  reinterpret_cast<utf8::String *>(clientLibrary_)->~String();
  mutex().~InterlockedMutex();
}
//---------------------------------------------------------------------------
utf8::String API::tryOpen()
{
#if defined(__WIN32__) || defined(__WIN64__)
  static const char libName[] = "odbc32.dll";
#else
  static const char libName[] = "libodbc.so";
#endif
  utf8::String libFileName(clientLibraryNL());
  if( libFileName.isNull() ) libFileName = libName;
  if( handle_ == NULL ){
    try {
#if defined(__WIN32__) || defined(__WIN64__)
      if( isWin9x() ){
        handle_ = LoadLibraryExA(anyPathName2HostPathName(libFileName).getANSIString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
      else {
        handle_ = LoadLibraryExW(anyPathName2HostPathName(libFileName).getUNICODEString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
#elif HAVE_DLFCN_H
      handle_ = dlopen(anyPathName2HostPathName(libFileName).getANSIString(),
#ifdef __linux__
        RTLD_GLOBAL | RTLD_NOW
#else
        0
#endif
      );
#endif
    }
    catch( ExceptionSP & ){
      if( handle_ != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
        FreeLibrary(handle_);
#elif HAVE_DLFCN_H
        dlclose(handle_);
#endif
        handle_ = NULL;
      }
    }
  }
  return libFileName;
}
//---------------------------------------------------------------------------
void API::open()
{
  AutoLock<InterlockedMutex> lock(mutex());
  if( count_ == 0 ){
    int32_t       err;
    utf8::String  libFileName (tryOpen());
    if( handle_ == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = GetLastError() + errorOffset;
#elif HAVE_DLFCN_H
      err = errno;
#endif
      stdErr.debug(
        9,
        utf8::String::Stream() << "Load " << libFileName << " failed\n"
      );
      newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
    for( uintptr_t i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
      void * & func = *((void **) &SQLAllocHandle + i);
#if defined(__WIN32__) || defined(__WIN64__)
      func = GetProcAddress(handle_,symbols_[i]);
      if( func == NULL ){
        err = GetLastError() + errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        stdErr.debug(
          9,
          utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
        );
        newObjectV1C2<Exception>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
      }
#elif HAVE_DLFCN_H
      func = dlsym(handle_,symbols_[i]);
      if( func == NULL ){
        err = errno;
        dlclose(handle_);
        handle_ = NULL;
        stdErr.debug(9,
	        utf8::String::Stream() << "dlsym(\"" << symbols_[i] << "\")\n"
	      );
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
  }
  count_++;
}
//---------------------------------------------------------------------------
void API::close()
{
  AutoLock<InterlockedMutex> lock(mutex());
  assert( count_ > 0 );
  if( count_ == 1 ){
#if defined(__WIN32__) || defined(__WIN64__)
    FreeLibrary(handle_);
#elif HAVE_DLFCN_H
    dlclose(handle_);
#endif
    handle_ = NULL;
  }
  count_--;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
