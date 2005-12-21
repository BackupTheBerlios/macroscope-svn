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
SockAddr & SockAddr::resolve(const utf8::String & addr,uintptr_t defPort)
{
#if HAVE_STRUCT_SOCKADDR_IN6
  memset(&addr6_,0,sizeof(addr6_));
#if HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN
  addr6_.sin6_len = sizeof(addr6_);
#endif
  addr6_.sin6_family = PF_INET6;
//  struct hostent * pent = api.gethostbyname2(host.getANSIString(),PF_INET6);
#else
  memset(&addr4_,0,sizeof(addr4_));
  addr4_.sin_family = PF_INET;
#endif
  unsigned a1 = 0, a2 = 0, a3 = 0, a4 = 0, port = (unsigned) defPort;
  int r = sscanf(addr.c_str(),"%u.%u.%u.%u:%u",&a1,&a2,&a3,&a4,&port);
  if( r >= 4 && a1 < 256 && a2 < 256 && a3 < 256 && a4 < 256 ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr4_.sin_len = sizeof(addr4_);
#endif
    addr4_.sin_family = PF_INET;
    char b[4 + 4 + 4 + 3 + 1];
    sprintf(b,"%u.%u.%u.%u",a1,a2,a3,a4);
    api.open();
    addr4_.sin_port = api.htons(port);
#if defined(__WIN32__) || defined(__WIN64__)
    addr4_.sin_addr.S_un.S_addr = api.inet_addr(b);
#else
    addr4_.sin_addr.s_addr = api.inet_addr(b);
#endif
    api.close();
  }
  else {
    utf8::String::Iterator pi(addr.strstr(utf8::string(":")));
    if( pi.position() < 0 ) pi.last();
    utf8::String host(utf8::string(utf8::String::Iterator(addr),pi));
    utf8::String port(utf8::string(pi + 1));
    host = host.trim();
    api.open();
    try {
      struct hostent * pent = api.gethostbyname(host.getANSIString());
      intmax_t a = defPort;
      if( pent == NULL || port.trim().strlen() == 0 || utf8::tryStr2Int(port,a) ){
        if( pent != NULL && host.strlen() > 0 ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
          addr4_.sin_len = sizeof(addr4_);
//          addr4_.sin_len = pent->h_length;
#endif
          addr4_.sin_family = pent->h_addrtype;
          memcpy(&addr4_.sin_addr,pent->h_addr_list[0],pent->h_length);
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
        int32_t err = errno;
        throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
      }
    }
    catch( ... ){
      api.close();
      throw;
    }
    api.close();
  }
  return *this;
}
//------------------------------------------------------------------------------
void SockAddr::resolve(const utf8::String & bind,ksys::Array<SockAddr> & addrs,uintptr_t defPort)
{
  intptr_t i = ksys::enumStringParts(bind);
  if( i <= 0 ){
    addrs.resize(1);
    addrs[0].resolve(utf8::String(),defPort);
  }
  else{
    addrs.resize(i);
    while( --i >= 0 )
      addrs[i].resolve(ksys::stringPartByNo(bind,i),defPort);
  }
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
