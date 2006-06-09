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
SockAddr & SockAddr::resolve(const utf8::String & addr,const ksys::Mutant & defPort)
{
#if HAVE_STRUCT_SOCKADDR_IN6
  memset(&addr6_, 0, sizeof(addr6_));
#if HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN
  addr6_.sin6_len = sizeof(addr6_);
#endif
  addr6_.sin6_family = PF_INET6;
  //  struct hostent * pent = api.gethostbyname2(host.getANSIString(),PF_INET6);
#else
  memset(&addr4_,0,sizeof(addr4_));
  addr4_.sin_family = PF_INET;
#endif
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
/*      struct hostent * pent = api.gethostbyname(host.getANSIString());
      intmax_t a = defPort;
      if( pent == NULL || port.trim().strlen() == 0 || utf8::tryStr2Int(port, a) ){
        if( pent != NULL && host.strlen() > 0 ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
          addr4_.sin_len = sizeof(addr4_);
          //          addr4_.sin_len = pent->h_length;
#endif
          addr4_.sin_family = pent->h_addrtype;
          memcpy(&addr4_.sin_addr, pent->h_addr_list[0], pent->h_length);
        }
        else {
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
          addr4_.sin_len = sizeof(addr4_);
#endif
          addr4_.sin_family = PF_INET;
#if defined(__WIN32__) || defined(__WIN64__)
          addr4_.sin_addr.S_un.S_addr = INADDR_ANY;
#else
          addr4_.sin_addr.s_addr = INADDR_ANY;
#endif
        }
        addr4_.sin_port = api.htons((u_short) a);
      }
      else {
        int32_t err = errNo();
        throw ksys::ExceptionSP(new EAsyncSocket(err, __PRETTY_FUNCTION__));
      }*/
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
          memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
        }
        api.FreeAddrInfoA(aiList);
      }
      else {
        for( resW = aiListW; resW != NULL; resW = resW->ai_next ){
          memcpy(&addr4_,resW->ai_addr,resW->ai_addrlen);
        }
        api.FreeAddrInfoW(aiListW);
      }
#else
      for( res = aiList; res != NULL; res = res->ai_next ){
        memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
      }
      api.freeaddrinfo(aiList);
#endif
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
utf8::String SockAddr::gethostname()
{
  int32_t err = 0;
  utf8::String s;
  api.open();
  while( api.gethostname(s.c_str(),(int) s.size()) != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( (err = errNo()) != WSAEFAULT + ksys::errorOffset) break;
#else
    if( (err = errNo()) != EFAULT ) break;
#endif
    s.resize((s.size() << 1) + (s.size() == 0));
    err = 0;
  }
  api.close();
  if( err != 0 )
    throw ksys::ExceptionSP(new EAsyncSocket(err + ksys::errorOffset,__PRETTY_FUNCTION__));
  return s.resize(s.size());
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
