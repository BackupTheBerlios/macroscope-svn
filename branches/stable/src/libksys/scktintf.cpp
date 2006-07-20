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
  "getsockname",
  "getpeername",
  "getaddrinfo",
  "freeaddrinfo",
  "getnameinfo",
  "GetAddrInfoW",
  "FreeAddrInfoW",
  "GetNameInfoW"
};

HINSTANCE API::handle_;

const char * const APIEx::symbols_[] = {
  "AcceptEx",
  "GetAcceptExSockaddrs"
};
HINSTANCE APIEx::handle_;
#endif
//---------------------------------------------------------------------------
WSADATA API::wsaData_;
uint8_t API::mutex_[sizeof(ksys::InterlockedMutex)];
uintptr_t API::count_;
APIEx apiEx;
//---------------------------------------------------------------------------
IPHLPAPI iphlpapi;
const char * const IPHLPAPI::symbols_[] = {
  "GetAdaptersAddresses"
};
HINSTANCE IPHLPAPI::handle_;
//---------------------------------------------------------------------------
WSHIP6API wship6api;
const char * const WSHIP6API::symbols_[] = {
  "getaddrinfo",
  "freeaddrinfo",
  "getnameinfo"
};
HINSTANCE WSHIP6API::handle_;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
API api;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
void API::open()
{
  int32_t err;

  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
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
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
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
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
      }
    }
    else {
      handle_ = LoadLibraryExW(L"ws2_32.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        ksys::stdErr.debug(9,utf8::String::Stream() << "Load 'ws2_32.dll' failed\n");
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
      }
      apiEx.handle_ = LoadLibraryExW(L"mswsock.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      if( apiEx.handle_ == NULL ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        ksys::stdErr.debug(9,utf8::String::Stream() << "Load 'mswsock.dll' failed\n");
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
      }
      if( ksys::isWinXPorLater() ){
        iphlpapi.handle_ = LoadLibraryExW(L"iphlpapi.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if( iphlpapi.handle_ == NULL ){
          err = GetLastError() + ksys::errorOffset;
          FreeLibrary(handle_);
          handle_ = NULL;
          FreeLibrary(apiEx.handle_);
          apiEx.handle_ = NULL;
          ksys::stdErr.debug(9,utf8::String::Stream() << "Load 'lphlpapi.dll' failed\n");
          ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
        }
      }
      /*if( ksys::isWin9x() || GetProcAddress(handle_,"GetAddrInfoW") == NULL ){
        wship6api.handle_ = LoadLibraryExA("wship6.dll",NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
        if( wship6api.handle_ == NULL ){
          err = GetLastError() + ksys::errorOffset;
          FreeLibrary(handle_);
          handle_ = NULL;
          FreeLibrary(apiEx.handle_);
          apiEx.handle_ = NULL;
          FreeLibrary(iphlpapi.handle_);
          iphlpapi.handle_ = NULL;
          ksys::stdErr.debug(9,utf8::String::Stream() << "Load 'wship6.dll' failed\n");
          Exception::throwSP(err, __PRETTY_FUNCTION__);
        }
      }*/
    }
    uintptr_t i;
    for( i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
      ((void **) &p_WSAGetLastError)[i] = GetProcAddress(handle_, symbols_[i]);
      if( ((void **) &p_WSAGetLastError)[i] == NULL &&
          (void **) &p_WSAGetLastError + i != &p_getaddrinfo &&
          (void **) &p_WSAGetLastError + i != &p_freeaddrinfo &&
          (void **) &p_WSAGetLastError + i != &p_getnameinfo &&
          (void **) &p_WSAGetLastError + i != &p_GetAddrInfoW &&
          (void **) &p_WSAGetLastError + i != &p_FreeAddrInfoW &&
          (void **) &p_WSAGetLastError + i != &p_GetNameInfoW ){
        err = GetLastError() + ksys::errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        FreeLibrary(apiEx.handle_);
        apiEx.handle_ = NULL;
        FreeLibrary(iphlpapi.handle_);
        iphlpapi.handle_ = NULL;
        FreeLibrary(wship6api.handle_);
        wship6api.handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
        );
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
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
        FreeLibrary(iphlpapi.handle_);
        iphlpapi.handle_ = NULL;
        FreeLibrary(wship6api.handle_);
        wship6api.handle_ = NULL;
        ksys::stdErr.log(
          ksys::lmERROR,
          utf8::String::Stream() << "GetProcAddress(\"" << apiEx.symbols_[i] << "\")\n"
        );
        ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
      }
    }
    if( ksys::isWinXPorLater() ){
      for( i = 0; i < sizeof(iphlpapi.symbols_) / sizeof(iphlpapi.symbols_[0]); i++ ){
        ((void **) &iphlpapi.p_GetAdaptersAddresses)[i] = GetProcAddress(iphlpapi.handle_,iphlpapi.symbols_[i]);
        if( ((void **) &iphlpapi.p_GetAdaptersAddresses)[i] == NULL ){
          err = GetLastError() + ksys::errorOffset;
          FreeLibrary(handle_);
          handle_ = NULL;
          FreeLibrary(apiEx.handle_);
          apiEx.handle_ = NULL;
          FreeLibrary(iphlpapi.handle_);
          iphlpapi.handle_ = NULL;
          ksys::stdErr.debug(9,utf8::String::Stream() <<
            "GetProcAddress(\"" << iphlpapi.symbols_[i] << "\")\n"
          );
          ksys::Exception::throwSP(err, __PRETTY_FUNCTION__);
        }
      }
    }
    /*if( ksys::isWin9x() || GetProcAddress(handle_,"GetAddrInfoW") == NULL ){
      for( i = 0; i < sizeof(wship6api.symbols_) / sizeof(wship6api.symbols_[0]); i++ ){
        ((void **) &wship6api.p_getaddrinfo)[i] = GetProcAddress(wship6api.handle_,wship6api.symbols_[i]);
        if( ((void **) &wship6api.p_getaddrinfo)[i] == NULL ){
          err = GetLastError() + ksys::errorOffset;
          FreeLibrary(handle_);
          handle_ = NULL;
          FreeLibrary(apiEx.handle_);
          apiEx.handle_ = NULL;
          FreeLibrary(iphlpapi.handle_);
          iphlpapi.handle_ = NULL;
          FreeLibrary(wship6api.handle_);
          wship6api.handle_ = NULL;
          ksys::stdErr.debug(9,utf8::String::Stream() <<
            "GetProcAddress(\"" << wship6api.symbols_[i] << "\")\n"
          );
          Exception::throwSP(err, __PRETTY_FUNCTION__);
        }
      }
    }*/
#endif
    if( this->WSAStartup(MAKEWORD(2,2), &wsaData_) != 0 ){
      err = this->WSAGetLastError() + ksys::errorOffset;
#ifndef USE_STATIC_SOCKET_LIBRARY
      FreeLibrary(handle_);
      handle_ = NULL;
      FreeLibrary(apiEx.handle_);
      apiEx.handle_ = NULL;
      FreeLibrary(iphlpapi.handle_);
      iphlpapi.handle_ = NULL;
      FreeLibrary(wship6api.handle_);
      wship6api.handle_ = NULL;
#endif
      ksys::stdErr.debug(9,utf8::String::Stream() << "WSAStartup failed\n");
      ksys::Exception::throwSP(err,"WSAStartup failed");
    }
    if( LOBYTE(wsaData_.wVersion) != 2 || HIBYTE(wsaData_.wVersion) != 2 ){
      ksys::stdErr.debug(9,utf8::String::Stream() <<
        "The Windows Sockets version requested (2.2) is not supported.\n"
      );
#ifndef USE_STATIC_SOCKET_LIBRARY
      FreeLibrary(handle_);
      handle_ = NULL;
      FreeLibrary(apiEx.handle_);
      apiEx.handle_ = NULL;
      FreeLibrary(iphlpapi.handle_);
      iphlpapi.handle_ = NULL;
      FreeLibrary(wship6api.handle_);
      wship6api.handle_ = NULL;
#endif
      ksys::Exception::throwSP(WSAVERNOTSUPPORTED,__PRETTY_FUNCTION__);
    }
  }
  count_++;
}
//---------------------------------------------------------------------------
void API::close()
{
  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
  assert( count_ > 0 );
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
#ifndef NDEBUG
    FreeLibrary(iphlpapi.handle_);
    iphlpapi.handle_ = NULL;
    FreeLibrary(wship6api.handle_);
    wship6api.handle_ = NULL;
    for( i = sizeof(iphlpapi.symbols_) / sizeof(iphlpapi.symbols_[0]) - 1; i >= 0; i-- )
      ((void **) &iphlpapi.p_GetAdaptersAddresses)[i] = NULL;
    for( i = sizeof(wship6api.symbols_) / sizeof(wship6api.symbols_[0]) - 1; i >= 0; i-- )
      ((void **) &wship6api.p_getaddrinfo)[i] = NULL;
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
