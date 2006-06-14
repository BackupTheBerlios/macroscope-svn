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
//------------------------------------------------------------------------------
namespace ksock {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//------------------------------------------------------------------------------
SockAddr & SockAddr::clear()
{
#if HAVE_STRUCT_SOCKADDR_IN6
  memset(&addr6_,0,sizeof(addr6_));
#if HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN
  addr6_.sin6_len = sizeof(addr6_);
#endif
  addr6_.sin6_family = PF_INET6;
#else
  memset(&addr4_,0,sizeof(addr4_));
  addr4_.sin_family = PF_INET;
#endif
  return *this;
}
//------------------------------------------------------------------------------
utf8::String SockAddr::internalGetAddrInfo(const utf8::String & host,const utf8::String & port,const ksys::Mutant & defPort,int ai_flag)
{
  utf8::String s;
#if defined(__WIN32__) || defined(__WIN64__)
  union {
    struct addrinfo aiHints;
    struct addrinfoW aiHintsW;
  };
  union {
    struct addrinfo * aiList;
    struct addrinfoW * aiListW;
  };
  union {
    struct addrinfo * res;
    struct addrinfoW * resW;
  };
#else
  struct addrinfo aiHints;
  struct addrinfo * aiList = NULL, * res;
#endif
  res = NULL;
  memset(&aiHints,0,sizeof(aiHints));
  aiHints.ai_family = PF_UNSPEC;
  aiHints.ai_socktype = SOCK_STREAM;
  aiHints.ai_protocol = IPPROTO_TCP;
  aiHints.ai_flags |= host.strlen() == 0 ? AI_PASSIVE : 0;
  aiHints.ai_flags |= ai_flag;
  int r;
#if defined(__WIN32__) || defined(__WIN64__)
  if( ksys::isWin9x() ){
    r = api.GetAddrInfoA(
      host.strlen() > 0 ? (const char *) host.getANSIString() : NULL,
      port.strlen() > 0 && (uintptr_t) defPort != 0 ? (const char *) port.getANSIString() : NULL,
      &aiHints,
      &aiList
    );
  }
  else {
    r = api.GetAddrInfoW(
      host.strlen() > 0 ? (const wchar_t *) host.getUNICODEString() : NULL,
      port.strlen() > 0 && (uintptr_t) defPort != 0 ? (const wchar_t *) port.getUNICODEString() : NULL,
      &aiHintsW,
      &aiListW
    );
  }
#else
  r = api.getaddrinfo(
    host.strlen() > 0 ? (const char *) host.getANSIString() : NULL,
    port.strlen() > 0 && (uintptr_t) defPort != 0 ? (const char *) port.getANSIString() : NULL,
    &aiHints,
    &aiList
  );
#endif
  if( r != 0 ){
    int32_t err = errNo();
    throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( ksys::isWin9x() ){
    for( res = aiList; res != NULL; res = res->ai_next ){
      if( res->ai_canonname != NULL ) s = res->ai_canonname;
      memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
    }
    api.FreeAddrInfoA(aiList);
  }
  else {
    for( resW = aiListW; resW != NULL; resW = resW->ai_next ){
      if( res->ai_canonname != NULL ) s = res->ai_canonname;
      memcpy(&addr4_,resW->ai_addr,resW->ai_addrlen);
    }
    api.FreeAddrInfoW(aiListW);
  }
#else
  for( res = aiList; res != NULL; res = res->ai_next ){
    if( res->ai_canonname != NULL ) s = res->ai_canonname;
    memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
  }
  api.freeaddrinfo(aiList);
#endif
  return s;
}
//------------------------------------------------------------------------------
SockAddr & SockAddr::resolve(const utf8::String & addr,const ksys::Mutant & defPort)
{
  clear();
/*  unsigned a1 = 0, a2 = 0, a3 = 0, a4 = 0, port = (unsigned) defPort;
  int r = sscanf(addr.c_str(), "%u.%u.%u.%u:%u", &a1, &a2, &a3, &a4, &port);
  if( r >= 4 && a1 < 256 && a2 < 256 && a3 < 256 && a4 < 256 ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr4_.sin_len = sizeof(addr4_);
#endif
    addr4_.sin_family = PF_INET;
    char  b[4 + 4 + 4 + 3 + 1];
    sprintf(b, "%u.%u.%u.%u", a1, a2, a3, a4);
    api.open();
    addr4_.sin_port = api.htons((u_short) port);
#if defined(__WIN32__) || defined(__WIN64__)
    addr4_.sin_addr.S_un.S_addr = api.inet_addr(b);
#else
    addr4_.sin_addr.s_addr = api.inet_addr(b);
#endif
    api.close();
  }
  else {*/
    utf8::String::Iterator  pi(addr.strstr(":"));
    if( pi.position() < 0 ) pi.last();
    utf8::String host(utf8::String::Iterator(addr), pi);
    utf8::String port(pi + 1);
    if( port.trim().strlen() == 0 ) port = defPort;
    host = host.trim();
    port = port.trim();
    api.open();
    try {
      internalGetAddrInfo(host,port,defPort,0);
    }
    catch( ... ){
      api.close();
      throw;
    }
    api.close();
  //}
  return *this;
}
//------------------------------------------------------------------------------
void SockAddr::resolve(const utf8::String & bind, ksys::Array<SockAddr> & addrs,const ksys::Mutant & defPort)
{
  intptr_t i = ksys::enumStringParts(bind);
  if( i <= 0 ){
    addrs.resize(1);
    addrs[0].resolve(utf8::String(),defPort);
  }
  else {
    for( intptr_t j = 0; j < i; j++ ){
      addrs.resize(j + 1);
      addrs[j].resolve(ksys::stringPartByNo(bind,j),defPort);
    }
  }
}
//------------------------------------------------------------------------------
utf8::String SockAddr::resolve() const
{
  int32_t err = 0;
  utf8::String s;
  api.open();
  struct hostent * pent = api.gethostbyaddr((const char *) &addr4_,(int) length(),addr4_.sin_family);
  if( pent != NULL ) s = pent->h_name; else err = errNo();
  api.close();
  if( err != 0 )
    throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
  return s;
}
//------------------------------------------------------------------------------
SockAddr & SockAddr::resolveAsync(const utf8::String & addr,const ksys::Mutant & defPort)
{
  assert( ksys::currentFiber() != NULL );
  ksys::currentFiber()->event_.string0_ = addr;
  ksys::currentFiber()->event_.string1_ = defPort;
  ksys::currentFiber()->event_.type_ = ksys::etResolveName;
  ksys::currentFiber()->thread()->postRequest();
  ksys::currentFiber()->switchFiber(ksys::currentFiber()->mainFiber());
  assert( ksys::currentFiber()->event_.type_ == ksys::etResolveName );
  if( ksys::currentFiber()->event_.errno_ != 0 )
    throw ksys::ExceptionSP(
      new EAsyncSocket(ksys::currentFiber()->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)
    );
  memcpy(&addr4_,&ksys::currentFiber()->event_.address_.addr4_,ksys::currentFiber()->event_.address_.length());
  return *this;
}
//------------------------------------------------------------------------------
void SockAddr::resolveAsync(const utf8::String & bind,ksys::Array<SockAddr> & addrs,const ksys::Mutant & defPort)
{
  intptr_t i = ksys::enumStringParts(bind);
  if( i <= 0 ){
    addrs.resize(1);
    addrs[0].resolveAsync(utf8::String(),defPort);
  }
  else {
    for( intptr_t j = 0; j < i; j++ ){
      addrs.resize(j + 1);
      addrs[j].resolve(ksys::stringPartByNo(bind,j),defPort);
    }
  }
}
//------------------------------------------------------------------------------
utf8::String SockAddr::resolveAsync() const
{
  assert( ksys::currentFiber() != NULL );
  ksys::currentFiber()->event_.address_ = *this;
  ksys::currentFiber()->event_.type_ = ksys::etResolveAddress;
  ksys::currentFiber()->thread()->postRequest();
  ksys::currentFiber()->switchFiber(ksys::currentFiber()->mainFiber());
  assert( ksys::currentFiber()->event_.type_ == ksys::etResolveAddress );
  if( ksys::currentFiber()->event_.errno_ != 0 )
    throw ksys::ExceptionSP(
      new EAsyncSocket(ksys::currentFiber()->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)
    );
  return ksys::currentFiber()->event_.string0_;
}
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
void SockAddr::getAdaptersAddresses(ksys::AutoPtr<IpInfo> & addresses)
{
  DWORD dwRetVal;
  ULONG outBufLen = 0;
  if( ksys::isWinXPorLater() ){
// Make an initial call to GetAdaptersAddresses to get the 
// size needed into the outBufLen variable
    dwRetVal = GetAdaptersAddresses(
      AF_UNSPEC,
      0, 
      NULL, 
      &addresses->addresses_,
      &outBufLen
    );
    if( dwRetVal == ERROR_BUFFER_OVERFLOW ){
      addresses.realloc(outBufLen);
    }
    else if( dwRetVal != ERROR_SUCCESS ){
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
    }
// Make a second call to GetAdapters Addresses to get the
// actual data we want
    dwRetVal = GetAdaptersAddresses(
      AF_UNSPEC,
      0, 
      NULL, 
      &addresses->addresses_,
      &outBufLen
    );
    if( dwRetVal != ERROR_SUCCESS ){
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
    }
  }
  else {
    dwRetVal = GetAdaptersInfo(NULL,&outBufLen);
    if( dwRetVal == ERROR_BUFFER_OVERFLOW ){
      addresses.realloc(outBufLen);
    }
    else if( dwRetVal != ERROR_SUCCESS ){
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
    }
    dwRetVal = GetAdaptersInfo(&addresses->infos_,&outBufLen);
    if( dwRetVal != ERROR_SUCCESS ){
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
    }
  }
}
#endif	
//------------------------------------------------------------------------------
/*utf8::String SockAddr::gethostname()
{
  int32_t err = 0;
  utf8::String s;
#if defined(__WIN32__) || defined(__WIN64__)
  union {
    char hostName[NI_MAXHOST];
    wchar_t hostNameW[NI_MAXHOST];
  };
  union {
    char servInfo[NI_MAXSERV];
    wchar_t servInfoW[NI_MAXSERV];
  };
  ksys::AutoPtr<IpInfo> addresses;
  getAdaptersAddresses(addresses);
  if( ksys::isWinXPorLater() ){
    IP_ADAPTER_ADDRESSES * pAddress = &addresses->addresses_;
    while( pAddress != NULL ){
// exclude loopback interface
      if( pAddress->PhysicalAddressLength > 0 ){
        PIP_ADAPTER_UNICAST_ADDRESS unicast = pAddress->FirstUnicastAddress;
        while( unicast != NULL ){
          SockAddr addr;
          addr.clear();
          for( intptr_t i = sizeof(pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_data) - 1; i >= 0; i-- ){
            ((uint8_t *) &addr.addr4_.sin_addr)[i] = 
              pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_data[sizeof(pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_data) - 1 - i];
          }
          addr.addr4_.sin_family = pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_family;
          err = api.GetNameInfoW(
            (const sockaddr *) &addr.addr4_,
            (socklen_t) addr.length(),
            hostNameW,
            sizeof(hostNameW),
            servInfoW,
            sizeof(servInfoW),
            NI_NUMERICSERV
          );
          if( err == 0 ) break;
          unicast = unicast->Next;
        }
      }
      if( err == 0 ) break;
      pAddress = pAddress->Next;
    }
    if( err == 0 ) s = hostNameW;
  }
  else {
  }
#else
  try {
    api.open();
    while( api.gethostname(s.c_str(),(int) s.size()) != 0 ){
      if( (err = errNo()) != EFAULT ) break;
      s.resize((s.size() << 1) + (s.size() == 0));
      err = 0;
    }
    if( err == 0 ){
      s.resize(s.size());
      err = api.getnameinfo(
        (const sockaddr *) &addr.addr4_,
        (socklen_t) addr.length(),
        hostName,
        sizeof(hostName),
        servInfo,
        sizeof(servInfo),
        NI_NUMERICSERV
      );
      if( err == 0 ) s = hostName;
    }
  }
  catch( ... ){
    api.close();
    throw;
  }
  api.close();
#endif
  if( err != 0 ){
    err = errNo() + ksys::errorOffset;
    throw ksys::ExceptionSP(new EAsyncSocket(err,__PRETTY_FUNCTION__));
  }
  return s;
}*/
//------------------------------------------------------------------------------
utf8::String SockAddr::gethostname()
{
  int32_t err = 0;
  utf8::String s;
  api.open();
  try {
    SockAddr addr;
#if defined(__WIN32__) || defined(__WIN64__)
    union {
      char hostName[NI_MAXHOST];
      wchar_t hostNameW[NI_MAXHOST];
    };
    union {
      char servInfo[NI_MAXSERV];
      wchar_t servInfoW[NI_MAXSERV];
    };
    ksys::AutoPtr<IpInfo> addresses;
    getAdaptersAddresses(addresses);
    if( ksys::isWinXPorLater() ){
      IP_ADAPTER_ADDRESSES * pAddress = &addresses->addresses_;
      while( pAddress != NULL ){
// exclude loopback interface
        if( pAddress->IfType == MIB_IF_TYPE_LOOPBACK ) continue;
        if( pAddress->PhysicalAddressLength == 0 ) continue;
        PIP_ADAPTER_UNICAST_ADDRESS unicast = pAddress->FirstUnicastAddress;
        while( unicast != NULL ){
          addr.clear();
          ksys::reverseByteArray(
            &addr.addr4_.sin_addr,
            pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_data,
            sizeof(pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_data)
          );
          addr.addr4_.sin_family = pAddress->FirstUnicastAddress->Address.lpSockaddr->sa_family;
          err = api.GetNameInfoW(
            (const sockaddr *) &addr.addr4_,
            (socklen_t) addr.length(),
            hostNameW,
            sizeof(hostNameW),
            servInfoW,
            sizeof(servInfoW),
            NI_NUMERICSERV
          );
          if( err == 0 ) break;
          unicast = unicast->Next;
        }
        if( err == 0 ) break;
        pAddress = pAddress->Next;
      }
      if( err == 0 ) s = hostNameW;
    }
    else {
      IP_ADAPTER_INFO * pAddress = &addresses->infos_;
      while( pAddress != NULL ){
        if( pAddress->Type == MIB_IF_TYPE_LOOPBACK ) continue;
        PIP_ADDR_STRING list = &pAddress->IpAddressList;
        while( list != NULL ){
          addr.internalGetAddrInfo(list->IpAddress.String,utf8::String(),0,0);
          if( ksys::isWin9x() ){
            err = api.GetNameInfoA(
              (const sockaddr *) &addr.addr4_,
              (socklen_t) addr.length(),
              hostName,
              sizeof(hostName),
              servInfo,
              sizeof(servInfo),
              NI_NUMERICSERV
            );
            if( err == 0 ) s = hostName;
          }
          else {
            err = api.GetNameInfoW(
              (const sockaddr *) &addr.addr4_,
              (socklen_t) addr.length(),
              hostNameW,
              sizeof(hostNameW),
              servInfoW,
              sizeof(servInfoW),
              NI_NUMERICSERV
            );
            if( err == 0 ) s = hostNameW;
          }
          if( err == 0 ) break;
          list = list->Next;
        }
        if( err == 0 ) break;
        pAddress = pAddress->Next;
      }
    }
#endif
    if( s.strlen() == 0 ){
      while( api.gethostname(s.c_str(),(socklen_t) s.size()) != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        if( (err = errNo()) != WSAEFAULT ) break;
#else
        if( (err = errNo()) != EFAULT ) break;
#endif
        s.resize((s.size() << 1) + (s.size() == 0));
        err = 0;
      }
      s.resize(s.size());
    }
#if !defined(__WIN32__) && !defined(__WIN64__)
    addr.internalGetAddrInfo(s,utf8::String(),0,0);
    err = api.getnameinfo(
      (const sockaddr *) &addr.addr4_,
      (socklen_t) addr.length(),
      hostName,
      sizeof(hostName),
      servInfo,
      sizeof(servInfo),
      NI_NUMERICSERV
    );
    if( err == 0 ) s = hostName;
#endif
    if( err != 0 ) err = errNo();
  }
  catch( ... ){
    api.close();
    throw;
  }
  api.close();
  if( err != 0 )
    throw ksys::ExceptionSP(new EAsyncSocket(err + ksys::errorOffset,__PRETTY_FUNCTION__));
  return s;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
