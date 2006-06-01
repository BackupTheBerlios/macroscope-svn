/*-
 * Copyright 2006 Guram Dukashvili
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
// msmail1cImpl.h : Declaration of the Cmsmail1c

#pragma once
#include "resource.h"       // main symbols
#include <adicpp/adicpp.h>
using namespace ksys;
#include "msmail1clib.h"
#include "../msmail/msmail.h"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientFiber : public ksock::ClientFiber {
  friend class Client;
  public:
    virtual ~ClientFiber();
    ClientFiber(Client & client);
  protected:
    Client & client_;
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();
    void main();
  private:
    ClientFiber(const ClientFiber & a) : client_(a.client_) {}
    void operator = (const ClientFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientMailSenderFiber : public ClientFiber {
  friend class Client;
  public:
    virtual ~ClientMailSenderFiber();
    ClientMailSenderFiber(Client & client,Message & message);
  protected:
    Message & message_;

    void main();
  private:
    ClientMailSenderFiber(const ClientMailSenderFiber & a,Message & message) :
      ClientFiber(a.client_), message_(message) {}
    void operator = (const ClientMailSenderFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : public ksock::Client {
  public:
    virtual ~Client();
    Client();

    void open();
    void close();

    IAsyncEvent * pAsyncEvent_;
    utf8::String name_;
    utf8::String user_;
    utf8::String key_;
    utf8::String groups_;
    utf8::String mailServer_;
    utf8::String configFile_;
    utf8::String logFile_;
    ConfigSP config_;

    const utf8::String & newMessage();
    const utf8::String & value(const utf8::String & id,const utf8::String & key) const;
    utf8::String value(const utf8::String & id,const utf8::String & key,const utf8::String & value);

    Client & sendMessage(const utf8::String & id);
    InterlockedMutex sendWait_;
    int32_t sendLastError_;
  protected:
    Vector<Message> recvQueue_;
    Vector<Message> sendQueue_;
  private:
    Client(const Client &) : config_(NULL) {}
    void operator = (const Client &){}
};
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Cmsmail1c
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ATL_NO_VTABLE Cmsmail1c :
  public CComObjectRootEx<CComMultiThreadModel>,
  public CComCoClass<Cmsmail1c, &CLSID_msmail1c>,
  public IDispatchImpl<Imsmail1c, &IID_Imsmail1c, &LIBID_msmail1clibLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
  public IDispatchImpl<IInitDone, &__uuidof(IInitDone), &LIBID_msmail1clibLib, /* wMajor = */ 1, /* wMinor = */ 0>,
  public IDispatchImpl<ILanguageExtender, &__uuidof(ILanguageExtender), &LIBID_msmail1clibLib, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
  virtual ~Cmsmail1c();
  Cmsmail1c();

  DECLARE_REGISTRY_RESOURCEID(IDR_MSMAIL1C)

  BEGIN_COM_MAP(Cmsmail1c)
    COM_INTERFACE_ENTRY(Imsmail1c)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IInitDone)
    COM_INTERFACE_ENTRY(ILanguageExtender)
  END_COM_MAP()

  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

public:
  msmail::Client client_;
  LPDISPATCH pBackConnection_;
  IAsyncEvent * pAsyncEvent_;
  utf8::String name_;
  utf8::String user_;
  utf8::String key_;
  utf8::String groups_;
  utf8::String mailServer_;
  utf8::String configFile_;
  utf8::String logFile_;
  bool active_;
  LONG lastError_;
// queue interface
  class LockedFile {
    public:
      HANDLE handle_;
      HANDLE hEvent_;
      utf8::String name_;
      DWORD lastError_;

      ~LockedFile();
      LockedFile();
  };
  Vector<LockedFile> files_;
  Randomizer rnd_;
  LockedFile * findFileByName(const utf8::String & name);
  LockedFile * addFile(const utf8::String & name);
  STDMETHOD(lockFile)(IN BSTR name,IN ULONG minSleepTime,IN ULONG maxSleepTime,OUT LONG * pLastError);
  STDMETHOD(unlockFile)(IN BSTR name,OUT LONG * pLastError);
  STDMETHOD(getLastError)(IN BSTR name,OUT LONG * pLastError);
  STDMETHOD(sleep)(IN LONG sleepTime);
  STDMETHOD(sleepIn)(IN ULONG minSleepTime,IN ULONG maxSleepTime);
  STDMETHOD(textToFile)(IN BSTR name,IN BSTR text,OUT LONG * pLastError);

  // IInitDone Methods
public:
  STDMETHOD(Init)( IDispatch * pConnection);
  STDMETHOD(Done)();
  STDMETHOD(GetInfo)(SAFEARRAY * * pInfo);
  // ILanguageExtender Methods
public:
  STDMETHOD(RegisterExtensionAs)( BSTR * bstrExtensionName);
  STDMETHOD(GetNProps)( long * plProps);
  STDMETHOD(FindProp)( BSTR bstrPropName,  long * plPropNum);
  STDMETHOD(GetPropName)( long lPropNum,  long lPropAlias,  BSTR * pbstrPropName);
  STDMETHOD(GetPropVal)( long lPropNum,  VARIANT * pvarPropVal);
  STDMETHOD(SetPropVal)( long lPropNum,  VARIANT * varPropVal);
  STDMETHOD(IsPropReadable)( long lPropNum,  BOOL * pboolPropRead);
  STDMETHOD(IsPropWritable)( long lPropNum,  BOOL * pboolPropWrite);
  STDMETHOD(GetNMethods)( long * plMethods);
  STDMETHOD(FindMethod)(BSTR bstrMethodName,  long * plMethodNum);
  STDMETHOD(GetMethodName)( long lMethodNum,  long lMethodAlias,  BSTR * pbstrMethodName);
  STDMETHOD(GetNParams)( long lMethodNum,  long * plParams);
  STDMETHOD(GetParamDefValue)( long lMethodNum,  long lParamNum,  VARIANT * pvarParamDefValue);
  STDMETHOD(HasRetVal)( long lMethodNum,  BOOL * pboolRetValue);
  STDMETHOD(CallAsProc)( long lMethodNum, SAFEARRAY * * paParams);
  STDMETHOD(CallAsFunc)( long lMethodNum,  VARIANT * pvarRetValue, SAFEARRAY * * paParams);
};

OBJECT_ENTRY_AUTO(__uuidof(msmail1c), Cmsmail1c)
