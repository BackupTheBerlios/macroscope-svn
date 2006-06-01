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
#ifndef _sockaddr_H_
//---------------------------------------------------------------------------
#define _sockaddr_H_
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class InAddr {
  public:
    union {
        struct in_addr  addr4_;
#if HAVE_STRUCT_SOCKADDR_IN6
        struct in6_addr addr6_;
#endif
    };
  protected:
  private:
};
//---------------------------------------------------------------------------
class SockAddr {
  public:
    union {
        struct sockaddr_in  addr4_;
#if HAVE_STRUCT_SOCKADDR_IN6
        struct sockaddr_in6 addr6_;
#endif
    };
    SockAddr & resolve(const utf8::String & addr,uintptr_t defPort = 0);
    static void resolve(const utf8::String & bind,ksys::Array<SockAddr> & addrs,uintptr_t defPort = 0);
    utf8::String resolve() const;

    SockAddr & resolveAsync(const utf8::String & addr,uintptr_t defPort = 0);
    static void resolveAsync(const utf8::String & bind,ksys::Array<SockAddr> & addrs,uintptr_t defPort = 0);
    utf8::String resolveAsync() const;

    static utf8::String gethostname();

    uintptr_t length() const;
  protected:
  private:
};
//---------------------------------------------------------------------------
inline uintptr_t SockAddr::length() const
{
  uintptr_t l = 0;
  //#if HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  //  return addr4_.sin_len;
  //#else
  if( addr4_.sin_family == PF_INET ) l = sizeof(addr4_);
#if HAVE_STRUCT_SOCKADDR_IN6
  else if( addr6_.sin6_family == PF_INET6 )
    l = sizeof(addr6_);
#endif
  //#endif
  return l;
}
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#endif /* _sockaddr_H_ */
//---------------------------------------------------------------------------

