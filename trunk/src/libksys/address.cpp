/*-
 * Copyright 2005-2008 Guram Dukashvili
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
SockAddr & SockAddr::clear(uintptr_t family)
{
#if SIZEOF_SOCKADDR_IN6
  memset(&addr6_,0,sizeof(addr6_));
#if SIZEOF_SOCKADDR_IN6_SIN6_LEN
  //addr6_.sin6_len = sizeof(addr6_);
#endif
  //addr6_.sin6_family = PF_INET6;
#else
  memset(&addr4_,0,sizeof(addr4_));
#endif
  addr4_.sin_family = uint8_t(family);
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
  aiHints.ai_flags = ai_flag;
  int r = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( (ksys::isWin9x() || api.GetAddrInfoW == NULL) && api.getaddrinfo != NULL ){
    utf8::AnsiString hp(host.getANSIString());
    utf8::AnsiString ap((port.isNull() ? (utf8::String) defPort : port).getANSIString());
    r = api.getaddrinfo(
      host.isNull() ? NULL : (const char *) hp,
      port.isNull() ? NULL : (const char *) ap,
      &aiHints,
      &aiList
    );
  }
  else if( api.getaddrinfo == NULL && api.GetAddrInfoW == NULL ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr4_.sin_len = sizeof(addr4_);
#endif
    addr4_.sin_family = PF_INET;
    if( !port.isNull() ){
      addr4_.sin_port = api.htons((u_short) utf8::str2Int(port));
    }
    else {
      addr4_.sin_port = api.htons(defPort);
    }
    if( !host.isNull() ){
      struct hostent * he = api.gethostbyname(host.getANSIString());
      if( he == NULL ){
        r = -1;
      }
      else {
        s = he->h_name;
        addr4_.sin_family = he->h_addrtype;
        memcpy(&addr4_.sin_addr,he->h_addr_list[0],he->h_length);
      }
    }
    else {
#if defined(__WIN32__) || defined(__WIN64__)
      addr4_.sin_addr.S_un.S_addr = INADDR_ANY;
#else
      addr4_.sin_addr.s_addr = INADDR_ANY;
#endif
    }
  }
  else {
    utf8::WideString hp(host.getUNICODEString());
    utf8::WideString ap((port.isNull() ? (utf8::String) defPort : port).getUNICODEString());
    r = api.GetAddrInfoW(
      host.isNull() ? NULL : (const wchar_t *) hp,
      port.isNull() ? NULL : (const wchar_t *) ap,
      &aiHintsW,
      &aiListW
    );
  }
#else
  utf8::AnsiString hp(host.getANSIString());
  utf8::AnsiString ap((port.isNull() ? (utf8::String) defPort : port).getANSIString());
  r = api.getaddrinfo(
    host.isNull() ? NULL : (const char *) hp,
    port.isNull() ? NULL : (const char *) ap,
    &aiHints,
    &aiList
  );
#endif
  if( r != 0 ){
    int32_t err = errNo();
    ksys::AutoPtr<EAsyncSocket> sp(newObjectV1C2<EAsyncSocket>(err,
      __PRETTY_FUNCTION__ + utf8::String(" ") + host + (!port.isNull() ? ":" + port : utf8::String())
    ));
#ifdef EAI_SYSTEM
    if( r != EAI_SYSTEM ){
      sp->addError(EINVAL," EAI " + utf8::int2Str(r));
      sp->code(0) = EINVAL;
    }
#endif
    sp.ptr(NULL)->throwSP();
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( (ksys::isWin9x() || api.GetAddrInfoW == NULL) && api.freeaddrinfo != NULL ){
    for( res = aiList; res != NULL; res = res->ai_next ){
      if( res->ai_canonname != NULL ) s = res->ai_canonname;
      if( addr4_.sin_family == res->ai_addr->sa_family )
        memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
    }
    api.freeaddrinfo(aiList);
  }
  else if( api.getaddrinfo == NULL && api.GetAddrInfoW == NULL ){
  }
  else {
    for( resW = aiListW; resW != NULL; resW = resW->ai_next ){
      if( res->ai_canonname != NULL ) s = res->ai_canonname;
      if( addr4_.sin_family == resW->ai_addr->sa_family )
        memcpy(&addr4_,resW->ai_addr,resW->ai_addrlen);
    }
    api.FreeAddrInfoW(aiListW);
  }
#else
  for( res = aiList; res != NULL; res = res->ai_next ){
    if( res->ai_canonname != NULL ) s = res->ai_canonname;
    if( addr4_.sin_family == res->ai_addr->sa_family )
      memcpy(&addr4_,res->ai_addr,res->ai_addrlen);
  }
  api.freeaddrinfo(aiList);
#endif
  return s;
}
//------------------------------------------------------------------------------
SockAddr & SockAddr::resolveName(const utf8::String & addr,const ksys::Mutant & defPort,intptr_t ai_flag)
{
  ksys::Fiber * fiber = ksys::currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.string0_ = addr;
    fiber->event_.string1_ = defPort;
    fiber->event_.aiFlag_ = int(ai_flag);
    fiber->event_.type_ = ksys::etResolveName;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == ksys::etResolveName );
    if( fiber->event_.errno_ != 0 )
      newObjectV1C2<EAsyncSocket>(fiber->event_.errno_,addr + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    memcpy(&addr4_,&((SockAddr *) fiber->event_.address_)->addr4_,((SockAddr *) fiber->event_.address_)->sockAddrSize());
  }
  else {
    clear();
    unsigned a1 = 0, a2 = 0, a3 = 0, a4 = 0, port = (unsigned) defPort;
    int r = sscanf(addr.c_str(), "%u.%u.%u.%u:%u", &a1, &a2, &a3, &a4, &port);
    if( r >= 4 && a1 < 256 && a2 < 256 && a3 < 256 && a4 < 256 ){
#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
      addr4_.sin_len = sizeof(addr4_);
#endif
      addr4_.sin_family = PF_INET;
      char  b[4 + 4 + 4 + 3 + 1];
      sprintf(b, "%u.%u.%u.%u", a1, a2, a3, a4);
      ksock::APIAutoInitializer ksockAPIAutoInitializer;
      addr4_.sin_port = api.htons((u_short) port);
#if defined(__WIN32__) || defined(__WIN64__)
      addr4_.sin_addr.S_un.S_addr = api.inet_addr(b);
#else
      addr4_.sin_addr.s_addr = api.inet_addr(b);
#endif
    }
    else {
      utf8::String::Iterator  pi(addr.strstr(":"));
      if( pi.position() < 0 ) pi.last();
      utf8::String host(utf8::String::Iterator(addr), pi);
      utf8::String port(pi + 1);
      if( port.trim().isNull() ) port = defPort;
      host = host.trim();
      port = port.trim();
      ksock::APIAutoInitializer ksockAPIAutoInitializer;
      internalGetAddrInfo(host,port,defPort,int(ai_flag));
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
void SockAddr::resolveNameForBind(const utf8::String & bind,ksys::Array<SockAddr> & addrs,const ksys::Mutant & defPort)
{
  intptr_t i = ksys::enumStringParts(bind);
  if( i <= 0 ){
    addrs.resize(1);
    addrs[0].resolveName(utf8::String(),defPort,AI_PASSIVE);
  }
  else {
    for( intptr_t j = 0; j < i; j++ ){
      addrs.resize(j + 1);
      addrs[j].resolveName(ksys::stringPartByNo(bind,j),defPort,AI_PASSIVE);
    }
  }
}
//------------------------------------------------------------------------------
uint8_t SockAddr::reverseResolveOverrideHolder_[sizeof(ksys::Array<ReverseResolveOverride>)];
uint8_t SockAddr::reverseResolveOverrideReadWriteLockHolder_[sizeof(ksys::WriteLock)];
//------------------------------------------------------------------------------
void SockAddr::initialize()
{
  new (reverseResolveOverrideHolder_) ksys::Array<ReverseResolveOverride>;
  new (reverseResolveOverrideReadWriteLockHolder_) ksys::WriteLock;
}
//------------------------------------------------------------------------------
void SockAddr::cleanup()
{
  using namespace ksys;
  reverseResolveOverrideReadWriteLock().~WriteLock();
  reverseResolveOverride().~Array<ReverseResolveOverride>();
}
//------------------------------------------------------------------------------
void SockAddr::reverseResolveOverrideAdd(const SockAddr & addr,const utf8::String & name)
{
  ksys::AutoLock<ksys::WriteLock> lock(reverseResolveOverrideReadWriteLock());
  ReverseResolveOverride ovr(addr,name);
  ovr.addr_.addr4_.sin_port = 0;
  intptr_t i = reverseResolveOverride().search(ovr);
  if( i < 0 ) reverseResolveOverride().add(ovr);
}
//------------------------------------------------------------------------------
void SockAddr::reverseResolveOverrideRemove(const SockAddr & addr)
{
  ksys::AutoLock<ksys::WriteLock> lock(reverseResolveOverrideReadWriteLock());
  ReverseResolveOverride ovr(addr);
  ovr.addr_.addr4_.sin_port = 0;
  intptr_t i = reverseResolveOverride().search(ovr);
  if( i < 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
    int32_t err = ERROR_NOT_FOUND;
#else
    int32_t err = ENOENT;
#endif
    newObjectV1C2<ksys::Exception>(err + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  reverseResolveOverride().remove(i);
}
//------------------------------------------------------------------------------
utf8::String SockAddr::reverseResolveGetOverride(const SockAddr & addr,const ksys::Mutant & defPort)
{
  ksys::AutoLock<ksys::WriteLock> lock(reverseResolveOverrideReadWriteLock());
  ReverseResolveOverride ovr(addr);
  ovr.addr_.addr4_.sin_port = 0;
  intptr_t i = reverseResolveOverride().search(ovr);
  utf8::String s;
  if( i >= 0 ){
    s = reverseResolveOverride()[i].name_.unique();
    if( addr.addr4_.sin_port != 0 ){
      ksys::Mutant m(defPort);
      if( !m.tryChangeType(ksys::mtInt) || be16toh(addr.addr4_.sin_port) != (intmax_t) m )
        s += ":" + utf8::int2Str(be16toh(addr.addr4_.sin_port));
    }
  }
  return s;
}
//------------------------------------------------------------------------------
utf8::String SockAddr::resolveAddr(const ksys::Mutant & defPort,intptr_t aiFlag) const
{
  ksys::Fiber * fiber = ksys::currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    memcpy(fiber->event_.address_,this,sizeof(*this));
    fiber->event_.defPort_ = defPort;
    fiber->event_.aiFlag_ = aiFlag;
    fiber->event_.type_ = ksys::etResolveAddress;
    fiber->thread()->postRequest();
    fiber->switchFiber(ksys::currentFiber()->mainFiber());
    assert( fiber->event_.type_ == ksys::etResolveAddress );
    if( fiber->event_.errno_ != 0 )
      newObjectV1C2<EAsyncSocket>(fiber->event_.errno_,__PRETTY_FUNCTION__)->throwSP();
    return fiber->event_.string0_;
  }
  intmax_t serv;
  int32_t err = 0;
  utf8::String s = reverseResolveGetOverride(*this,defPort);
  if( !s.isNull() ) return s;
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
#if defined(__WIN32__) || defined(__WIN64__)
  union {
    char hostName[NI_MAXHOST];
    wchar_t hostNameW[NI_MAXHOST];
  };
  union {
    char servInfo[NI_MAXSERV];
    wchar_t servInfoW[NI_MAXSERV];
  };
  if( (ksys::isWin9x() || api.GetNameInfoW == NULL) && api.getnameinfo != NULL ){
    err = api.getnameinfo(
      (const sockaddr *) &addr4_,
      sockAddrSize(),
      hostName,
      sizeof(hostName),
      servInfo,
      sizeof(servInfo),
      int(aiFlag)
    );
    if( err == 0 ){
      s = hostName;
      if( addr4_.sin_port != 0 && (!utf8::tryStr2Int(servInfo,serv) || (intmax_t) defPort != serv) ){
        s += ":";
        s += servInfo;
      }
    }
  }
  else if( api.getnameinfo == NULL || api.GetNameInfoW == NULL ){
    struct hostent * he = api.gethostbyaddr(
      (const char *) &addr4_.sin_addr,
      addrSize(),
      addr4_.sin_family
    );
    if( he == NULL ){
      err = -1;
    }
    else {
      s = he->h_name;
      if( addr4_.sin_port != 0 && (	intptr_t) defPort != api.ntohs(addr4_.sin_port) ){
        s += ":";
        s += utf8::int2Str(api.ntohs(addr4_.sin_port));
      }
    }
  }
  else {
    err = api.GetNameInfoW(
      (const sockaddr *) &addr4_,
      (socklen_t) sockAddrSize(),
      hostNameW,
      sizeof(hostNameW),
      servInfoW,
      sizeof(servInfoW),
      int(aiFlag)
    );
    if( err == 0 ){
      s = hostNameW;
      if( addr4_.sin_port != 0 && (!utf8::tryStr2Int(servInfoW,serv) || (intmax_t) defPort != serv) ){
        s += ":";
        s += servInfoW;
      }
    }
  }
#else
  char hostName[NI_MAXHOST];
  char servInfo[NI_MAXSERV];
  err = api.getnameinfo(
    (const sockaddr *) &addr4_,
    (socklen_t) sockAddrSize(),
    hostName,
    sizeof(hostName),
    servInfo,
    sizeof(servInfo),
    int(aiFlag)
  );
  if( err == 0 ){
    s = hostName;
    if( addr4_.sin_port != 0 && (!utf8::tryStr2Int(servInfo,serv) || (intmax_t) defPort != serv) ){
      s += ":";
      s += utf8::int2Str(api.ntohs(addr4_.sin_port));
    }
  }
#endif
  if( err != 0 ){
    int32_t er = errNo();
    ksys::AutoPtr<EAsyncSocket> sp(newObjectV1C2<EAsyncSocket>(er,__PRETTY_FUNCTION__));
#ifdef EAI_SYSTEM
    if( err != EAI_SYSTEM ){
      sp->addError(EINVAL,"EAI " + utf8::int2Str(err));
      sp->code(0) = EINVAL;
    }
#endif
    sp.ptr(NULL)->throwSP();
  }
  return s;
}
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
void SockAddr::getAdaptersAddresses(ksys::Array<IpInfo> & addresses)
{
  DWORD dwRetVal;
  ULONG outBufLen;
  addresses.resize((ULONG) getpagesize() * 16u / sizeof(IpInfo));
  if( ksys::isWinXPorLater() || iphlpapi.GetAdaptersAddresses != NULL ){
    for(;;){
      outBufLen = (ULONG) (addresses.count() * sizeof(IpInfo));
      dwRetVal = iphlpapi.GetAdaptersAddresses(
        AF_UNSPEC,
        0, 
        NULL, 
        &addresses[0].addresses_,
        &outBufLen
      );
      if( dwRetVal == ERROR_SUCCESS ) break;
      if( dwRetVal == ERROR_BUFFER_OVERFLOW ){
        if( addresses.count() * sizeof(IpInfo) < (ULONG(1) << (sizeof(ULONG) * CHAR_BIT - 1)) ){
          addresses.resize(addresses.count() << 1);
          continue;
        }
      }
      newObjectV1C2<ksys::Exception>(dwRetVal + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  else {
    for(;;){
      outBufLen = (ULONG) (addresses.count() * sizeof(IpInfo));
      dwRetVal = iphlpapi.GetAdaptersInfo(&addresses[0].infos_,&outBufLen);
      if( dwRetVal == ERROR_SUCCESS ) break;
      if( dwRetVal == ERROR_BUFFER_OVERFLOW ){
        if( addresses.count() * sizeof(IpInfo) < (ULONG(1) << (sizeof(ULONG) * CHAR_BIT - 1)) ){
          addresses.resize(addresses.count() << 1);
          continue;
        }
      }
      newObjectV1C2<ksys::Exception>(dwRetVal + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
    }
  }
}
#endif	
//------------------------------------------------------------------------------
utf8::String SockAddr::gethostname(bool noThrow,const utf8::String & def)
{
  SockAddr addr;
  int32_t err = 0;
  IP_ADAPTER_ADDRESSES * pAddress = NULL;
  IP_ADAPTER_INFO * pAddress2 = NULL;
  PIP_ADAPTER_UNICAST_ADDRESS unicast = NULL;
  PIP_ADDR_STRING list = NULL;
  utf8::String s;
  APIAutoInitializer ksockAPIAutoInitializer;
#if defined(__WIN32__) || defined(__WIN64__)
  ksys::Array<IpInfo> addresses;
  getAdaptersAddresses(addresses);
  if( ksys::isWinXPorLater() || iphlpapi.GetAdaptersAddresses != NULL ){
    pAddress = &addresses[0].addresses_;
    while( pAddress != NULL ){
// exclude loopback interface
      if( pAddress->IfType == MIB_IF_TYPE_LOOPBACK ) continue;
      if( pAddress->PhysicalAddressLength == 0 ) continue;
      unicast = pAddress->FirstUnicastAddress;
      while( unicast != NULL ){
#ifdef IP_ADAPTER_ADDRESS_PRIMARY
        if( unicast->Flags & (IP_ADAPTER_ADDRESS_DNS_ELIGIBLE | IP_ADAPTER_ADDRESS_PRIMARY) ){
#else
        if( unicast->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE ){
#endif
          addr.clear();
            //ksys::reverseByteArray(
          memcpy(
            &addr.addr4_,
            unicast->Address.lpSockaddr,
            unicast->Address.iSockaddrLength
          );
            //addr.addr4_.sin_family = unicast->Address.lpSockaddr->sa_family;
          try {
            err = 0;
            s = addr.resolveAddr();
          }
          catch( ksys::ExceptionSP & e ){
            err = e->code() >= ksys::errorOffset ? e->code() - ksys::errorOffset : e->code();
          }
          if( err == 0 ) break;
        }
        unicast = unicast->Next;
      }
      if( err == 0 ) break;
      pAddress = pAddress->Next;
    }
  }
  else {
    pAddress2 = &addresses[0].infos_;
    while( pAddress2 != NULL ){
      if( pAddress2->Type == MIB_IF_TYPE_LOOPBACK ) continue;
      list = &pAddress2->IpAddressList;
      while( list != NULL ){
        try {
          err = 0;
          s = addr.resolveName(list->IpAddress.String).resolveAddr();
        }
        catch( ksys::ExceptionSP & e ){
          err = e->code() >= ksys::errorOffset ? e->code() - ksys::errorOffset : e->code();
        }
        if( err == 0 ) break;
        list = list->Next;
      }
      if( err == 0 ) break;
      pAddress2 = pAddress2->Next;
    }
  }
#endif
  if( s.isNull() ){
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
#if !defined(__WIN32__) && !defined(__WIN64__)
    addr.internalGetAddrInfo(s,utf8::String(),0,0);
    char hostName[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    err = api.getnameinfo(
      (const sockaddr *) &addr.addr4_,
      (socklen_t) addr.sockAddrSize(),
      hostName,
      sizeof(hostName),
      servInfo,
      sizeof(servInfo),
      NI_NUMERICSERV
    );
    if( err == 0 ) s = hostName;
    if( err != 0 ) err = errNo();
#endif
  }
  if( err != 0 ){
    if( noThrow ){
      s = def;
    }
    else {
      int32_t er = errNo();
      ksys::AutoPtr<EAsyncSocket> sp(newObjectV1C2<EAsyncSocket>(er,__PRETTY_FUNCTION__));
#ifdef EAI_SYSTEM
      if( err != EAI_SYSTEM ){
        sp->addError(EINVAL,"EAI " + utf8::int2Str(err));
        sp->code(0) = EINVAL;
      }
#endif
      sp.ptr(NULL)->throwSP();
    }
  }
  return s;
}
//------------------------------------------------------------------------------
utf8::String SockAddr::addr2Index(const struct in_addr & addr)
{
  ksys::AutoPtr<char,AutoPtrMemoryDestructor> b;
  b.alloc(sizeof(addr.s_addr) * 2 + 1);
  const uint8_t * ip = (const uint8_t *) &addr;
  uintptr_t i;
  for( i = 0; i < sizeof(addr.s_addr); i++ )
  for( intptr_t j = 1; j >= 0; j-- ) b[i * 2 + 1 - j] = "0123456789ABCDEF"[(ip[(i * 8 + j * 4) >> 3] >> (j * 4)) & 0xF];
  b[i * 2] = '\0';
  return utf8::plane0(b);
}
//------------------------------------------------------------------------------
#if SIZEOF_SOCKADDR_IN6
//------------------------------------------------------------------------------
utf8::String SockAddr::addr2Index(const struct in6_addr & addr)
{
  ksys::AutoPtr<char,AutoPtrMemoryDestructor> b;
  b.alloc(sizeof(addr) * 2 + 1);
  const uint8_t * ip = (const uint8_t *) &addr;
  uintptr_t i;
  for( i = 0; i < sizeof(addr); i++ )
  for( intptr_t j = 1; j >= 0; j-- ) b[i * 2 + 1 - j] = "0123456789ABCDEF"[(ip[(i * 8 + j * 4) >> 3] >> (j * 4)) & 0xF];
  b[i * 2] = '\0';
  return utf8::plane0(b);
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#if SIZEOF_SOCKADDR_DL
//------------------------------------------------------------------------------
utf8::String SockAddr::saddr2Index(const struct sockaddr_dl & addr)
{
  return link_ntoa(&addr);
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
utf8::String SockAddr::addr2Index() const
{
  if( addr4_.sin_family == AF_INET ) return addr2Index(addr4_.sin_addr);
#if SIZEOF_SOCKADDR_IN6
  else if( addr6_.sin6_family == AF_INET6 ) return addr2Index(addr6_.sin6_addr);
#endif
#if SIZEOF_SOCKADDR_DL
  else if( addrDL_.sdl_family == AF_LINK ) return saddr2Index(addrDL_);
#endif
  else
    newObjectV1C2<ksys::Exception>(ENOSYS,__PRETTY_FUNCTION__);
  return utf8::String();
}
//---------------------------------------------------------------------------
struct in_addr SockAddr::indexToAddr4(const utf8::String & index)
{
  struct in_addr addr;
  *(uint32_t *) &addr = be32toh((uint32_t) utf8::str2Int(index,16));
  return addr;
}
//------------------------------------------------------------------------------
#if SIZEOF_SOCKADDR_IN6
//------------------------------------------------------------------------------
struct in6_addr SockAddr::indexToAddr6(const utf8::String & /*index*/)
{
  struct in6_addr addr;
  memset(&addr,0,sizeof(addr));
  newObjectV1C2<ksys::Exception>(ENOSYS,__PRETTY_FUNCTION__);
  return addr;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif
#ifndef AF_ROUTE
#define AF_ROUTE 17
#endif
#ifndef AF_LINK
#define AF_LINK 18
#endif
#ifndef AF_COIP
#define AF_COIP 20
#endif
#ifndef AF_CNT
#define AF_CNT 21
#endif
#ifndef AF_ISDN
#define AF_ISDN 26
#endif
#ifndef AF_NATM
#define AF_NATM 29
#endif
#ifndef AF_SLOW
#define AF_SLOW 33
#endif
#ifndef AF_SCLUSTER
#define AF_SCLUSTER 34
#endif
#ifndef AF_SIP
#define AF_SIP 24
#endif
#ifndef AF_ARP
#define AF_ARP 35
#endif
#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 36
#endif
#ifndef AF_NETGRAPH
#define AF_NETGRAPH 32
#endif
//---------------------------------------------------------------------------
utf8::String SockAddr::addressFamilyAsString(uintptr_t family)
{
  const char * s;
  switch( family ){
    case AF_UNSPEC      : s = "UNSPEC"; break;
    case AF_LOCAL       : s = "LOCAL"; break;
    case AF_INET        : s = "INET"; break;
#ifdef AF_IMPLINK
    case AF_IMPLINK     : s = "IMPLINK"; break;
#endif
#ifdef AF_PUP
    case AF_PUP         : s = "PUP"; break;
#endif
#ifdef AF_CHAOS
    case AF_CHAOS       : s = "CHAOS"; break;
#endif
#ifdef AF_NETBIOS
    case AF_NETBIOS     : s = "NETBIOS"; break;
#endif
#ifdef AF_ISO
    case AF_ISO         : s = "ISO"; break;
#endif
#ifdef AF_ECMA
    case AF_ECMA        : s = "ECMA"; break;
#endif
#ifdef AF_DATAKIT
    case AF_DATAKIT     : s = "DATAKIT"; break;
#endif
#ifdef AF_CCITT
    case AF_CCITT       : s = "CCITT"; break;
#endif
    case AF_SNA         : s = "SNA"; break;
    case AF_DECnet      : s = "DECnet"; break;
#ifdef AF_DLI
    case AF_DLI         : s = "DLI"; break;
#endif
#ifdef AF_LAT
    case AF_LAT         : s = "LAT"; break;
#endif
#ifdef AF_HYLINK
    case AF_HYLINK      : s = "HYLINK"; break;
#endif
    case AF_APPLETALK   : s = "APPLETALK"; break;
#if !defined(__WIN32__) && !defined(__WIN64__)
    case AF_ROUTE       : s = "ROUTE"; break;
#endif
    case AF_LINK        : s = "LINK"; break;
    case AF_COIP        : s = "COIP"; break;
    case AF_CNT         : s = "CNT"; break;
    case AF_SIP         : s = "SIP"; break;
    case AF_IPX         : s = "IPX"; break;
    case AF_ISDN        : s = "ISDN"; break;
    case AF_INET6       : s = "INET6"; break;
    case AF_NATM        : s = "ATM"; break;
#ifdef AF_ATM
    case AF_ATM         : s = "ATM"; break;
#endif
    case AF_NETGRAPH    : s = "NETGRAPH"; break;
    case AF_SLOW        : s = "SLOW"; break;
    case AF_SCLUSTER    : s = "SCLUSTER"; break;
    case AF_ARP         : s = "ARP"; break;
    case AF_BLUETOOTH   : s = "BLUETOOTH"; break;
    default             :
      return utf8::int2Str(family);
  }
  return s;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
#define IPPROTO_ST              7               /* Stream protocol II */
#define IPPROTO_EGP             8               /* exterior gateway protocol */
#define IPPROTO_PIGP            9               /* private interior gateway */
#define IPPROTO_RCCMON          10              /* BBN RCC Monitoring */
#define IPPROTO_NVPII           11              /* network voice protocol*/
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_ARGUS           13              /* Argus */
#define IPPROTO_EMCON           14              /* EMCON */
#define IPPROTO_XNET            15              /* Cross Net Debugger */
#define IPPROTO_CHAOS           16              /* Chaos*/
#define IPPROTO_MUX             18              /* Multiplexing */
#define IPPROTO_MEAS            19              /* DCN Measurement Subsystems */
#define IPPROTO_HMP             20              /* Host Monitoring */
#define IPPROTO_PRM             21              /* Packet Radio Measurement */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_TRUNK1          23              /* Trunk-1 */
#define IPPROTO_TRUNK2          24              /* Trunk-2 */
#define IPPROTO_LEAF1           25              /* Leaf-1 */
#define IPPROTO_LEAF2           26              /* Leaf-2 */
#define IPPROTO_RDP             27              /* Reliable Data */
#define IPPROTO_IRTP            28              /* Reliable Transaction */
#define IPPROTO_TP              29              /* tp-4 w/ class negotiation */
#define IPPROTO_BLT             30              /* Bulk Data Transfer */
#define IPPROTO_NSP             31              /* Network Services */
#define IPPROTO_INP             32              /* Merit Internodal */
#define IPPROTO_SEP             33              /* Sequential Exchange */
#define IPPROTO_3PC             34              /* Third Party Connect */
#define IPPROTO_IDPR            35              /* InterDomain Policy Routing */
#define IPPROTO_XTP             36              /* XTP */
#define IPPROTO_DDP             37              /* Datagram Delivery */
#define IPPROTO_CMTP            38              /* Control Message Transport */
#define IPPROTO_TPXX            39              /* TP++ Transport */
#define IPPROTO_IL              40              /* IL transport protocol */
#define IPPROTO_IPV6            41              /* IP6 header */
#define IPPROTO_SDRP            42              /* Source Demand Routing */
#define IPPROTO_ROUTING         43              /* IP6 routing header */
#define IPPROTO_FRAGMENT        44              /* IP6 fragmentation header */
#define IPPROTO_IDRP            45              /* InterDomain Routing*/
#define IPPROTO_RSVP            46              /* resource reservation */
#define IPPROTO_GRE             47              /* General Routing Encap. */
#define IPPROTO_MHRP            48              /* Mobile Host Routing */
#define IPPROTO_BHA             49              /* BHA */
#define IPPROTO_ESP             50              /* IP6 Encap Sec. Payload */
#define IPPROTO_INLSP           52              /* Integ. Net Layer Security */
#define IPPROTO_SWIPE           53              /* IP with encryption */
#define IPPROTO_NHRP            54              /* Next Hop Resolution */
#define IPPROTO_MOBILE          55              /* IP Mobility */
#define IPPROTO_TLSP            56              /* Transport Layer Security */
#define IPPROTO_SKIP            57              /* SKIP */
#define IPPROTO_ICMPV6          58              /* ICMP6 */
#define IPPROTO_NONE            59              /* IP6 no next header */
#define IPPROTO_DSTOPTS         60              /* IP6 destination option */
#define IPPROTO_AHIP            61              /* any host internal protocol */
#define IPPROTO_CFTP            62              /* CFTP */
#define IPPROTO_HELLO           63              /* "hello" routing protocol */
#define IPPROTO_SATEXPAK        64              /* SATNET/Backroom EXPAK */
#define IPPROTO_KRYPTOLAN       65              /* Kryptolan */
#define IPPROTO_RVD             66              /* Remote Virtual Disk */
#define IPPROTO_IPPC            67              /* Pluribus Packet Core */
#define IPPROTO_ADFS            68              /* Any distributed FS */
#define IPPROTO_SATMON          69              /* Satnet Monitoring */
#define IPPROTO_VISA            70              /* VISA Protocol */
#define IPPROTO_IPCV            71              /* Packet Core Utility */
#define IPPROTO_CPNX            72              /* Comp. Prot. Net. Executive */
#define IPPROTO_CPHB            73              /* Comp. Prot. HeartBeat */
#define IPPROTO_WSN             74              /* Wang Span Network */
#define IPPROTO_PVP             75              /* Packet Video Protocol */
#define IPPROTO_BRSATMON        76              /* BackRoom SATNET Monitoring */
#define IPPROTO_ND              77              /* Sun net disk proto (temp.) */
#define IPPROTO_WBMON           78              /* WIDEBAND Monitoring */
#define IPPROTO_WBEXPAK         79              /* WIDEBAND EXPAK */
#define IPPROTO_EON             80              /* ISO cnlp */
#define IPPROTO_VMTP            81              /* VMTP */
#define IPPROTO_SVMTP           82              /* Secure VMTP */
#define IPPROTO_VINES           83              /* Banyon VINES */
#define IPPROTO_TTP             84              /* TTP */
#define IPPROTO_IGP             85              /* NSFNET-IGP */
#define IPPROTO_DGP             86              /* dissimilar gateway prot. */
#define IPPROTO_TCF             87              /* TCF */
#define IPPROTO_IGRP            88              /* Cisco/GXS IGRP */
#define IPPROTO_OSPFIGP         89              /* OSPFIGP */
#define IPPROTO_SRPC            90              /* Strite RPC protocol */
#define IPPROTO_LARP            91              /* Locus Address Resoloution */
#define IPPROTO_MTP             92              /* Multicast Transport */
#define IPPROTO_AX25            93              /* AX.25 Frames */
#define IPPROTO_IPEIP           94              /* IP encapsulated in IP */
#define IPPROTO_MICP            95              /* Mobile Int.ing control */
#define IPPROTO_SCCSP           96              /* Semaphore Comm. security */
#define IPPROTO_ETHERIP         97              /* Ethernet IP encapsulation */
#define IPPROTO_ENCAP           98              /* encapsulation header */
#define IPPROTO_APES            99              /* any private encr. scheme */
#define IPPROTO_GMTP            100             /* GMTP*/
#define IPPROTO_IPCOMP          108             /* payload compression (IPComp) */
#define IPPROTO_SCTP            132             /* SCTP */
#define IPPROTO_PIM             103             /* Protocol Independent Mcast */
#define IPPROTO_CARP            112             /* CARP */
#define IPPROTO_PGM             113             /* PGM */
#define IPPROTO_PFSYNC          240             /* PFSYNC */
#elif !defined IPPROTO_SCTP
#define IPPROTO_SCTP            132             /* SCTP */
#endif
static const struct {
  const char * name_;
  uint8_t proto_;
} protos[] = {
  {          "", IPPROTO_IP        },
  {      "ICMP", IPPROTO_ICMP      },
  {      "IGMP", IPPROTO_IGMP      },
#ifdef IPPROTO_GGP
  {       "GGP", IPPROTO_GGP       },
#endif
#ifdef IPPROTO_IPV4
  {      "IPV4", IPPROTO_IPV4      },
#endif
  {       "TCP", IPPROTO_TCP       },
#ifdef IPPROTO_ST
  {        "ST", IPPROTO_ST        },
#endif
  {       "EGP", IPPROTO_EGP       },
#ifdef IPPROTO_PPPIGP
  {      "PIGP", IPPROTO_PIGP      },
#endif
#ifdef IPPROTO_RCCMON
  {    "RCCMON", IPPROTO_RCCMON    },
#endif
#ifdef IPPROTO_NVPII
  {     "NVPII", IPPROTO_NVPII     },
#endif
  {       "PUP", IPPROTO_PUP       },
#ifdef IPPROTO_ARGUS
  {     "ARGUS", IPPROTO_ARGUS     },
#endif
#ifdef IPPROTO_EMCON
  {     "EMCON", IPPROTO_EMCON     },
#endif
#ifdef IPPROTO_XNET
  {      "XNET", IPPROTO_XNET      },
#endif
#ifdef IPPROTO_CHAOS
  {     "CHAOS", IPPROTO_CHAOS     },
#endif
  {       "UDP", IPPROTO_UDP       },
#ifdef IPPROTO_MUX
  {       "MUX", IPPROTO_MUX       },
#endif
#ifdef IPPROTO_MEAS
  {      "MEAS", IPPROTO_MEAS      },
#endif
#ifdef IPPROTO_HMP
  {       "HMP", IPPROTO_HMP       },
#endif
#ifdef IPPROTO_PRM
  {       "PRM", IPPROTO_PRM       },
#endif
  {       "IDP", IPPROTO_IDP       },
#ifdef IPPROTO_TRUNK1
  {    "TRUNK1", IPPROTO_TRUNK1    },
#endif
#ifdef IPPROTO_TRUNK2
  {    "TRUNK2", IPPROTO_TRUNK2    },
#endif
#ifdef IPPROTO_LEAF1
  {     "LEAF1", IPPROTO_LEAF1     },
#endif
#ifdef IPPROTO_LEAF2
  {     "LEAF2", IPPROTO_LEAF2     },
#endif
#ifdef IPPROTO_RDP
  {       "RDP", IPPROTO_RDP       },
#endif
#ifdef IPPROTO_IRTP
  {      "IRTP", IPPROTO_IRTP      },
#endif
  {        "TP", IPPROTO_TP        },
#ifdef IPPROTO_BLT
  {       "BLT", IPPROTO_BLT       },
#endif
#ifdef IPPROTO_NSP
  {       "NSP", IPPROTO_NSP       },
#endif
#ifdef IPPROTO_INP
  {       "INP", IPPROTO_INP       },
#endif
#ifdef IPPROTO_SEP
  {       "SEP", IPPROTO_SEP       },
#endif
#ifdef IPPROTO_3PC
  {       "3PC", IPPROTO_3PC       },
#endif
#ifdef IPPROTO_IDPR
  {      "IDPR", IPPROTO_IDPR      },
#endif
#ifdef IPPROTO_XTP
  {       "XTP", IPPROTO_XTP       },
#endif
#ifdef IPPROTO_DDP
  {       "DDP", IPPROTO_DDP       },
#endif
#ifdef IPPROTO_CMTP
  {      "CMTP", IPPROTO_CMTP      },
#endif
#ifdef IPPROTO_TPXX
  {      "TPXX", IPPROTO_TPXX      },
#endif
#ifdef IPPROTO_IL
  {        "IL", IPPROTO_IL        },
#endif
  {      "IPV6", IPPROTO_IPV6      },
#ifdef IPPROTO_XTP
  {      "SDRP", IPPROTO_SDRP      },
#endif
  {   "ROUTING", IPPROTO_ROUTING   },
  {  "FRAGMENT", IPPROTO_FRAGMENT  },
#ifdef IPPROTO_XTP
  {      "IDRP", IPPROTO_IDRP      },
#endif
  {      "RSVP", IPPROTO_RSVP      },
  {       "GRE", IPPROTO_GRE       },
#ifdef IPPROTO_MHRP
  {      "MHRP", IPPROTO_MHRP      },
#endif
#ifdef IPPROTO_BHA
  {       "BHA", IPPROTO_BHA       },
#endif
  {       "ESP", IPPROTO_ESP       },
  {        "AH", IPPROTO_AH        },
#ifdef IPPROTO_INLSP
  {     "INLSP", IPPROTO_INLSP     },
#endif
#ifdef IPPROTO_SWIPE
  {     "SWIPE", IPPROTO_SWIPE     },
#endif
#ifdef IPPROTO_NHRP
  {      "NHRP", IPPROTO_NHRP      },
#endif
#ifdef IPPROTO_MOBILE
  {    "MOBILE", IPPROTO_MOBILE    },
#endif
#ifdef IPPROTO_TLSP
  {      "TLSP", IPPROTO_TLSP      },
#endif
#ifdef IPPROTO_SKIP
  {      "SKIP", IPPROTO_SKIP      },
#endif
  {    "ICMPV6", IPPROTO_ICMPV6    },
  {      "NONE", IPPROTO_NONE      },
  {   "DSTOPTS", IPPROTO_DSTOPTS   },
#ifdef IPPROTO_AHIP
  {      "AHIP", IPPROTO_AHIP      },
#endif
#ifdef IPPROTO_CFTP
  {      "CFTP", IPPROTO_CFTP      },
#endif
#ifdef IPPROTO_HELLO
  {     "HELLO", IPPROTO_HELLO     },
#endif
#ifdef IPPROTO_SATEXPAK
  {   "SATEXPAK", IPPROTO_SATEXPAK  },
#endif
#ifdef IPPROTO_KRYPROLAN
  { "KRYPTOLAN", IPPROTO_KRYPTOLAN },
#endif
#ifdef IPPROTO_RVD
  {       "RVD", IPPROTO_RVD       },
#endif
#ifdef IPPROTO_IPPC
  {      "IPPC", IPPROTO_IPPC      },
#endif
#ifdef IPPROTO_ADFS
  {      "ADFS", IPPROTO_ADFS      },
#endif
#ifdef IPPROTO_SATMON
  {    "SATMON", IPPROTO_SATMON    },
#endif
#ifdef IPPROTO_VISA
  {      "VISA", IPPROTO_VISA      },
#endif
#ifdef IPPROTO_IPCV
  {      "IPCV", IPPROTO_IPCV      },
#endif
#ifdef IPPROTO_CPHX
  {      "CPNX", IPPROTO_CPNX      },
#endif
#ifdef IPPROTO_CPHB
  {      "CPHB", IPPROTO_CPHB      },
#endif
#ifdef IPPROTO_WSN
  {       "WSN", IPPROTO_WSN       },
#endif
#ifdef IPPROTO_PVP
  {       "PVP", IPPROTO_PVP       },
#endif
#ifdef IPPROTO_BRSATMON
  {  "BRSATMON", IPPROTO_BRSATMON  },
#endif
#ifdef IPPROTO_ND
  {        "ND", IPPROTO_ND        },
#endif
#ifdef IPPROTO_WBMON
  {     "WBMON", IPPROTO_WBMON     },
#endif
#ifdef IPPROTO_WBEXPAK
  {   "WBEXPAK", IPPROTO_WBEXPAK   },
#endif
#ifdef IPPROTO_EON
  {       "EON", IPPROTO_EON       },
#endif
#ifdef IPPROTO_VMTP
  {      "VMTP", IPPROTO_VMTP      },
#endif
#ifdef IPPROTO_SVMTP
  {     "SVMTP", IPPROTO_SVMTP     },
#endif
#ifdef IPPROTO_VINES
  {     "VINES", IPPROTO_VINES     },
#endif
#ifdef IPPROTO_TTP
  {       "TTP", IPPROTO_TTP       },
#endif
#ifdef IPPROTO_IGP
  {       "IGP", IPPROTO_IGP       },
#endif
#ifdef IPPROTO_DGP
  {       "DGP", IPPROTO_DGP       },
#endif
#ifdef IPPROTO_TCF
  {       "TCF", IPPROTO_TCF       },
#endif
#ifdef IPPROTO_IGRP
  {      "IGRP", IPPROTO_IGRP      },
#endif
#ifdef IPPROTO_OSPFIGF
  {   "OSPFIGP", IPPROTO_OSPFIGP   },
#endif
#ifdef IPPROTO_SRPC
  {      "SRPC", IPPROTO_SRPC      },
#endif
#ifdef IPPROTO_LARP
  {      "LARP", IPPROTO_LARP      },
#endif
#ifdef IPPROTO_MTP
  {       "MTP", IPPROTO_MTP       },
#endif
#ifdef IPPROTO_AX25
  {      "AX25", IPPROTO_AX25      },
#endif
#ifdef IPPROTO_IPEIP
  {     "IPEIP", IPPROTO_IPEIP     },
#endif
#ifdef IPPROTO_MICP
  {      "MICP", IPPROTO_MICP      },
#endif
#ifdef IPPROTO_SCCSP
  {     "SCCSP", IPPROTO_SCCSP     },
#endif
#ifdef IPPROTO_ETHERIP
  {   "ETHERIP", IPPROTO_ETHERIP   },
#endif
  {     "ENCAP", IPPROTO_ENCAP     },
#ifdef IPPROTO_APES
  {      "APES", IPPROTO_APES      },
#endif
#ifdef IPPROTO_GMTP
  {      "GMTP", IPPROTO_GMTP      },
#endif
#ifdef IPPROTO_IPCOMP
  {    "IPCOMP", IPPROTO_IPCOMP    },
#endif
  {      "SCTP", IPPROTO_SCTP      },
  {       "PIM", IPPROTO_PIM       },
#ifdef IPPROTO_CARP
  {      "CARP", IPPROTO_CARP      },
#endif
#ifdef IPPROTO_PGM
  {       "PGM", IPPROTO_PGM       },
#endif
#ifdef IPPROTO_PFSYNC
  {    "PFSYNC", IPPROTO_PFSYNC    }
#endif
};
utf8::String SockAddr::protoAsString(uintptr_t proto)
{
  for( intptr_t i = sizeof(protos) / sizeof(protos[0]) - 1; i >= 0; i-- )
    if( uintptr_t(protos[i].proto_) == proto ) return protos[i].name_;
  return utf8::int2Str(proto);
}
//------------------------------------------------------------------------------
uintptr_t SockAddr::stringAsProto(const utf8::String & proto)
{
  for( intptr_t i = sizeof(protos) / sizeof(protos[0]) - 1; i >= 0; i-- )
    if( proto.casecompare(protos[i].name_) == 0 ) return protos[i].proto_;
  return (uintptr_t) utf8::str2Int(proto);
}
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
