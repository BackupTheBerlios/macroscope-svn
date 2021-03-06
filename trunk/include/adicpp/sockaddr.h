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
#ifndef _sockaddr_H_
//---------------------------------------------------------------------------
#define _sockaddr_H_
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class InAddr {
  public:
    union {
      struct in_addr  addr4_;
#if SIZEOF_SOCKADDR_IN6
      struct in6_addr addr6_;
#endif
    };
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ReverseResolveOverride;
//---------------------------------------------------------------------------
class SockAddr {
  friend void ksys::initialize(int argc,char ** argv);
  friend void ksys::cleanup();
  public:
    ~SockAddr() {}
    SockAddr() { clear(); }
    union {
      struct sockaddr_in  addr4_;
#if SIZEOF_SOCKADDR_IN6
      struct sockaddr_in6 addr6_;
#endif
#if SIZEOF_SOCKADDR_DL
      struct sockaddr_dl addrDL_;
#endif
    };

    operator struct sockaddr_in * () { return &addr4_; }
    operator const struct sockaddr_in * () const { return &addr4_; }
    operator struct sockaddr_in & () { return addr4_; }
    operator const struct sockaddr_in & () const { return addr4_; }
#if SIZEOF_SOCKADDR_IN6
    operator struct sockaddr_in6 * () { return &addr6_; }
    operator const struct sockaddr_in6 * () const { return &addr6_; }
    operator struct sockaddr_in6 & () { return addr6_; }
    operator const struct sockaddr_in6 & () const { return addr6_; }
#endif

    bool operator == (const SockAddr & addr) const;

    SockAddr & resolveName(const utf8::String & addr,const ksys::Mutant & defPort = 0,intptr_t ai_flag = 0);
    static void resolveNameForBind(const utf8::String & bind,ksys::Array<SockAddr> & addrs,const ksys::Mutant & defPort = 0);
    utf8::String resolveAddr(const ksys::Mutant & defPort = 0,intptr_t aiFlag = NI_NUMERICSERV) const;

    static utf8::String gethostname(bool noThrow = false,const utf8::String & def = utf8::String());

    socklen_t sockAddrSize() const;
    socklen_t addrSize() const;

    SockAddr & clear(uintptr_t family = PF_INET);
    
    static utf8::String addr2Index(const struct in_addr & addr);
    static utf8::String saddr2Index(const struct sockaddr_in & addr);
    static struct in_addr indexToAddr4(const utf8::String & index);
#if SIZEOF_SOCKADDR_IN6
    static utf8::String addr2Index(const struct in6_addr & addr);
    static utf8::String saddr2Index(const struct sockaddr_in6 & addr);
    static struct in6_addr indexToAddr6(const utf8::String & index);
#endif
#if SIZEOF_SOCKADDR_DL
    static struct sockaddr_dl indexToSAddrDL(const utf8::String & index);
    static utf8::String saddr2Index(const struct sockaddr_dl & addr);
#endif
    utf8::String addr2Index() const;
    utf8::String saddr2Index() const;
    static utf8::String addressFamilyAsString(uintptr_t family);
    utf8::String addressFamilyAsString() const { return addressFamilyAsString(addr4_.sin_family); }
    static utf8::String protoAsString(uintptr_t proto);
    static uintptr_t stringAsProto(const utf8::String & proto);

    static void reverseResolveOverrideAdd(const SockAddr & addr,const utf8::String & name);
    static void reverseResolveOverrideRemove(const SockAddr & addr);
    static utf8::String reverseResolveGetOverride(const SockAddr & addr,const ksys::Mutant & defPort);
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    union IpInfo {
      IP_ADAPTER_ADDRESSES addresses_;
      IP_ADAPTER_INFO infos_;
    };
    static void getAdaptersAddresses(ksys::Array<IpInfo> & addresses);
#endif
    utf8::String internalGetAddrInfo(
      const utf8::String & host,
      const utf8::String & port,
      const ksys::Mutant & defPort,
      int ai_flag
    );
  private:
    static uint8_t reverseResolveOverrideHolder_[];
    static uint8_t reverseResolveOverrideReadWriteLockHolder_[];

    static ksys::Array<ReverseResolveOverride> & reverseResolveOverride() {
      return *reinterpret_cast<ksys::Array<ReverseResolveOverride> *>(reverseResolveOverrideHolder_);
    }
    static ksys::WriteLock & reverseResolveOverrideReadWriteLock() {
      return *reinterpret_cast<ksys::WriteLock *>(reverseResolveOverrideReadWriteLockHolder_);
    }

    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline socklen_t SockAddr::sockAddrSize() const
{
  socklen_t l = 0;
  if( addr4_.sin_family == AF_INET ) l = sizeof(addr4_);
#if SIZEOF_SOCKADDR_IN6
  else if( addr6_.sin6_family == AF_INET6 ) l = sizeof(addr6_);
#endif
#if SIZEOF_SOCKADDR_DL
  else if( addrDL_.sdl_family == AF_LINK ) l = sizeof(addrDL_);
#endif
  return l;
}
//---------------------------------------------------------------------------
inline socklen_t SockAddr::addrSize() const
{
  socklen_t l = 0;
  if( addr4_.sin_family == AF_INET ) l = sizeof(addr4_.sin_addr);
#if SIZEOF_SOCKADDR_IN6
  else if( addr6_.sin6_family == AF_INET6 ) l = sizeof(addr6_.sin6_addr);
#endif
#if SIZEOF_SOCKADDR_DL
  else if( addrDL_.sdl_family == AF_LINK ) l = sizeof(addrDL_);
#endif
  return l;
}
//---------------------------------------------------------------------------
inline bool SockAddr::operator == (const SockAddr & addr) const
{
  return 
    addr4_.sin_family == addr.addr4_.sin_family &&
    addr4_.sin_port == addr.addr4_.sin_port &&
    memcmp(&addr4_.sin_addr,&addr.addr4_.sin_addr,addrSize()) == 0
  ;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ReverseResolveOverride {
  public:
    SockAddr addr_;
    utf8::String name_;

    ReverseResolveOverride() {}
    ReverseResolveOverride(const SockAddr & addr,const utf8::String & name = utf8::String()) :
      addr_(addr), name_(name) {}

    bool operator == (const ReverseResolveOverride & ovr) const { return addr_ == ovr.addr_; }
};
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option pop
#endif
//---------------------------------------------------------------------------
#endif /* _sockaddr_H_ */
//---------------------------------------------------------------------------
