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
#if __BCPLUSPLUS__
#pragma option -w-8084
#endif
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
#ifndef USE_STATIC_SOCKET_LIBRARY
const char * const  API::symbols_[]   = {
  "WSAGetLastError",
  "WSASetLastError",
  "WSAStartup",
  "WSACleanup",
  "WSAAsyncSelect",
  "socket",
  "closesocket",
  "shutdown",
  "bind",
  "listen",
  "accept",
  "recv",
  "recvfrom",
  "send",
  "sendto",
  "getsockopt",
  "setsockopt",
  "connect",
  "htonl",
  "ntohl",
  "htons",
  "ntohs",
  "inet_ntoa",
  "inet_addr",
  "WSARecv",
  "WSASend",
  "WSASocketA",
  "WSASocketW",
  "WSAGetOverlappedResult",
  "gethostbyname",
  "gethostbyaddr",
  "gethostname",
  "WSAEventSelect",
  "WSAEnumNetworkEvents",
  "WSAAsyncGetHostByName",
  "getaddrinfo",
  "GetAddrInfoW",
  "freeaddrinfo",
  "FreeAddrInfoW",
  "getnameinfo",
  "GetNameInfoW",
  "getsockname"
};

HINSTANCE           API::handle_;

const char * const  APIEx::symbols_[] = {
  "AcceptEx",
  "GetAcceptExSockaddrs"
};

HINSTANCE           APIEx::handle_;
#endif
//---------------------------------------------------------------------------
WSADATA             API::wsaData_;
APIEx               apiEx;
uint8_t             API::mutex_[sizeof(ksys::InterlockedMutex)];
uintptr_t           API::count_;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
API                 api;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
void API::open()
{
  int32_t                                 err;

  ksys::AutoLock< ksys::InterlockedMutex> lock  (mutex());
  if( count_ == 0 ){
#ifndef USE_STATIC_SOCKET_LIBRARY
    if( ksys::isWin9x() ){
      handle_ = LoadLibraryExA("ws2_32.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "Load 'ws2_32.dll' failed\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
      apiEx.handle_ = LoadLibraryExA("mswsock.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( apiEx.handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "Load 'mswsock.dll' failed\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
    }
    else{
      handle_ = LoadLibraryExW(L"ws2_32.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "Load 'ws2_32.dll' failed\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
      apiEx.handle_ = LoadLibraryExW(L"mswsock.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( apiEx.handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "Load 'mswsock.dll' failed\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
    }
    uintptr_t i;
    for( i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
      ((void **) &p_WSAGetLastError)[i] = GetProcAddress(handle_, symbols_[i]);
      if( ((void **) &p_WSAGetLastError)[i] == NULL ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        FreeLibrary(apiEx.handle_);
        apiEx.handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
    }
    for( i = 0; i < sizeof(apiEx.symbols_) / sizeof(apiEx.symbols_[0]); i++ ){
      ((void **) &apiEx.p_AcceptEx)[i] = GetProcAddress(apiEx.handle_, apiEx.symbols_[i]);
      if( ((void **) &apiEx.p_AcceptEx)[i] == NULL ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        FreeLibrary(apiEx.handle_);
        apiEx.handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "GetProcAddress(\"" << apiEx.symbols_[i] << "\")\n"
        );
        throw ksys::ExceptionSP(new ksys::Exception(err, __PRETTY_FUNCTION__));
      }
    }
#endif
    if( this->WSAStartup(MAKEWORD(2,2), &wsaData_) != 0 ){
      err = this->WSAGetLastError() + ksys::errorOffset;
#ifndef USE_STATIC_SOCKET_LIBRARY
      FreeLibrary(handle_);
      handle_ = NULL;
      FreeLibrary(apiEx.handle_);
      apiEx.handle_ = NULL;
#endif
      ksys::stdErr.log(
        ksys::lmERROR,
        utf8::String::Stream() << "WSAStartup failed\n"
      );
      throw ksys::ExceptionSP(new ksys::Exception(err,"WSAStartup failed"));
    }
    if( LOBYTE(wsaData_.wVersion) != 2 || HIBYTE(wsaData_.wVersion) != 2 ){
      ksys::stdErr.log(ksys::lmERROR,utf8::String::Stream() << "The Windows Sockets version requested (2.2) is not supported.\n");
      throw ksys::ExceptionSP(new ksys::Exception(WSAVERNOTSUPPORTED,__PRETTY_FUNCTION__));
    }

  }
  count_++;
}
//---------------------------------------------------------------------------
void API::close()
{
  assert(count_ > 0);
  ksys::AutoLock< ksys::InterlockedMutex> lock  (mutex());
  if( count_ == 1 ){
    int r = this->WSACleanup();
    assert( r == 0 );
    if( r != 0 ) abort();
#ifndef USE_STATIC_SOCKET_LIBRARY
    FreeLibrary(handle_);
    handle_ = NULL;
    FreeLibrary(apiEx.handle_);
    apiEx.handle_ = NULL;
#ifndef NDEBUG
    intptr_t i;
    for( i = sizeof(symbols_) / sizeof(symbols_[0]) - 1; i >= 0; i-- )
      ((void **) &p_WSAGetLastError)[i] = NULL;
    for( i = sizeof(apiEx.symbols_) / sizeof(apiEx.symbols_[0]) - 1; i >= 0; i-- )
      ((void **) &apiEx.p_AcceptEx)[i] = NULL;
#endif
#endif
  }
  count_--;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
void API::initialize()
{
#if defined(__WIN32__) || defined(__WIN64__)
  new (mutex_) ksys::InterlockedMutex;
  count_ = 0;
#ifndef USE_STATIC_SOCKET_LIBRARY
  handle_ = NULL;
  apiEx.handle_ = NULL;
#endif
#endif
}
//---------------------------------------------------------------------------
void API::cleanup()
{
#if defined(__WIN32__) || defined(__WIN64__)
  using ksys::InterlockedMutex;
  mutex().~InterlockedMutex();
#endif
}
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
