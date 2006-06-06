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
#ifndef _fbsvc_H_
#define _fbsvc_H_
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class SPB {
  friend class Service;
  friend class ServiceRequest;
  private:
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED SPBParam {
        const char *  name_;
        short         number;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
    static SPBParam params[];
    char *          spb_;
    uintptr_t       spbLen_;

    SPB &     writeChar(uintptr_t code);
    SPB &     writeLong(uintptr_t a);
    intptr_t  writeISCCode(const utf8::String & name);
    SPB &     writeBuffer(const void * buf, uintptr_t size);
  public:
    SPB();
    ~SPB();

    SPB &     clear();
    SPB &     add(const utf8::String & name, const ksys::Mutant & value = ksys::Mutant());
    // properties
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ServiceRequest {
  friend class Service;
  private:
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED RequestParam {
        const char *  name_;
        char          number;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
    static RequestParam params[];
    char *              request_;
    uintptr_t           requestLen_;
    uintptr_t           requestAction_;

    ServiceRequest &  writeChar(uintptr_t code);
    ServiceRequest &  writeShort(short a);
    ServiceRequest &  writeLong(uintptr_t a);
    intptr_t          writeISCCode(const utf8::String & name);
    ServiceRequest &  writeBuffer(const void * buf, uintptr_t size);
  public:
                      ServiceRequest();
                      ~ServiceRequest();

    ServiceRequest &  clear();
    ServiceRequest &  add(const utf8::String & name, const ksys::Mutant & value = ksys::Mutant());
    // properties
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Service : virtual public Base {
  public:
    virtual               ~Service();
                          Service();

    Service &             attach(const utf8::String & name);
    Service &             detach();
    Service &             invoke();

    // properties
    SPB &                 params();
    ServiceRequest &      request();
    isc_svc_handle &      handle();
    bool                  attached();
    const utf8::String &  name();
  protected:
    void                  staticExceptionHandler(ksys::Exception * e);
    void                  exceptionHandler(ksys::Exception * e);
  private:
    isc_svc_handle  handle_;
    SPB             spb_;
    ServiceRequest  request_;
    utf8::String    name_;
    char *          queryResponse_;
    uintptr_t       queryResponseLen_;
};
//---------------------------------------------------------------------------
inline SPB & Service::params()
{
  return spb_;
}
//---------------------------------------------------------------------------
inline ServiceRequest & Service::request()
{
  return request_;
}
//---------------------------------------------------------------------------
inline isc_svc_handle & Service::handle()
{
  return handle_;
}
//---------------------------------------------------------------------------
inline bool Service::attached()
{
  return handle_ != 0;
}
//---------------------------------------------------------------------------
inline const utf8::String & Service::name()
{
  return name_;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbdb_H_ */
