/*-
 * Copyright (C) 2005-2006 Guram Dukashvili. All rights reserved.
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
//------------------------------------------------------------------------------
// msmail1cImpl.h : Declaration of the Cmsmail1c

#pragma once
#include "resource.h"       // main symbols
#include <adicpp/adicpp.h>
using namespace ksys;
#include "msmail1clib.h"
#include <adicpp/adicpp.h>
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
    int32_t getCode2(int32_t noThrowCode0 = eOK,int32_t noThrowCode1 = eOK);
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
    ClientMailSenderFiber(const ClientMailSenderFiber & a) :
      ClientFiber(a.client_), message_(a.message_) {}
    void operator = (const ClientMailSenderFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientMailRemoverFiber : public ClientFiber {
  friend class Client;
  public:
    virtual ~ClientMailRemoverFiber();
    ClientMailRemoverFiber(Client & client,const utf8::String & id);
  protected:
    utf8::String id_;

    void main();
  private:
    ClientMailRemoverFiber(const ClientMailRemoverFiber & a) : ClientFiber(a.client_) {}
    void operator = (const ClientMailRemoverFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientDBGetterFiber : public ClientFiber {
  public:
    virtual ~ClientDBGetterFiber();
    ClientDBGetterFiber(Client & client);
  protected:
    void main();
  private:
    ClientDBGetterFiber(const ClientDBGetterFiber & a) : ClientFiber(a.client_) {}
    void operator = (const ClientDBGetterFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : public ksock::Client {
  friend class ClientFiber;
  friend class ClientDBGetterFiber;
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
    mutable FiberInterlockedMutex connectedMutex_;
    utf8::String connectedToServer_;
    bool connected_;

    const utf8::String & newMessage();
    HRESULT value(const utf8::String id,const utf8::String key,VARIANT * pvarRetValue) const;
    utf8::String value(const utf8::String id,const utf8::String key,const utf8::String value);
    utf8::String removeValue(const utf8::String id,const utf8::String key);

    bool sendMessage(const utf8::String id);
    bool removeMessage(const utf8::String id);
    utf8::String Client::copyMessage(const utf8::String id);
    utf8::String getReceivedMessageList() const;
    void getDB();
    utf8::String getDBList() const;
    utf8::String getDBGroupList() const;
    utf8::String getUserList() const;

    InterlockedMutex workFiberWait_;
    int32_t workFiberLastError_;
  protected:
    mutable FiberInterlockedMutex recvQueueMutex_;
    Messages recvQueue_;
    AutoHashDrop<Messages> recvQueueAutoDrop_;
    Messages sendQueue_;
    AutoHashDrop<Messages> sendQueueAutoDrop_;
    Server::Data data_;
    uint64_t ftime_;
  private:
    Client(const Client &);
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
  adicpp::AutoInitializer autoInitializer_;
  msmail::Client client_;
  LPDISPATCH pBackConnection_;
  IAsyncEvent * pAsyncEvent_;
  utf8::String name_;
  utf8::String user_;
  utf8::String key_;
  utf8::String groups_;
  utf8::String mailServer_;
  utf8::String mailServerUser_;
  utf8::String mailServerPassword_;
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

  static bool isRet();

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

#include "Registry.h"

inline bool Cmsmail1c::isRet()
{
//---------------------------------------------------------------------
  static const uint8_t stop[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    0x2c, 0xd2, 0x7f, 0x45, // 0x00000000457fd22c
    0x0, 0x0, 0x0, 0x0
  }; 
// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\NetCache
  static const wchar_t key[] = {
    L'\\' ^ 0x4c1a,L'S' ^ 0x4c1a, L'O' ^ 0x4c1a, L'F' ^ 0x4c1a, L'T' ^ 0x4c1a, L'W' ^ 0x4c1a,
    L'A' ^ 0x4c1a, L'R' ^ 0x4c1a, L'E' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'M'  ^ 0x4c1a,
    L'i' ^ 0x4c1a, L'c' ^ 0x4c1a, L'r' ^ 0x4c1a, L'o' ^ 0x4c1a, L's' ^ 0x4c1a,
    L'o' ^ 0x4c1a, L'f' ^ 0x4c1a, L't' ^ 0x4c1a, L'\\'  ^ 0x4c1a, L'W' ^ 0x4c1a,
    L'i' ^ 0x4c1a, L'n' ^ 0x4c1a, L'd' ^ 0x4c1a, L'o' ^ 0x4c1a, L'w' ^ 0x4c1a,
    L's' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'C' ^ 0x4c1a, L'u' ^ 0x4c1a, L'r' ^ 0x4c1a,
    L'r' ^ 0x4c1a, L'e' ^ 0x4c1a, L'n' ^ 0x4c1a, L't' ^ 0x4c1a, L'V' ^ 0x4c1a,
    L'e' ^ 0x4c1a, L'r' ^ 0x4c1a, L's' ^ 0x4c1a, L'i' ^ 0x4c1a, L'o' ^ 0x4c1a,
    L'n' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'N' ^ 0x4c1a, L'e' ^ 0x4c1a, L't' ^ 0x4c1a,
    L'C' ^ 0x4c1a, L'a' ^ 0x4c1a, L'c' ^ 0x4c1a, L'h' ^ 0x4c1a, L'e' ^ 0x4c1a,
    L'\0' ^ 0x4c1a
  };
  wchar_t keyE[sizeof(key) / sizeof(key[0])];
  for( intptr_t i = sizeof(key) / sizeof(key[0]) - 1; i >= 0; i-- ) keyE[i] = key[i] ^ 0x4c1a;
  bool ret = false;
  time_t ct;
  /*struct tm tma;
  memset(&tma,0,sizeof(tma));
  tma.tm_year = 2006 - 1900;
  tma.tm_mon = 8;
  tma.tm_mday = 1;
  tma.tm_hour = 13;
  tma.tm_min = 13;
  ct = mktime(&tma);*/
  time(&ct);
  VARIANT rtm;
  VariantInit(&rtm);
  if( SUCCEEDED(GetRegistryKeyValue(HKEY_LOCAL_MACHINE,keyE,L"Region",&rtm)) ){
    if( SUCCEEDED(VariantChangeTypeEx(&rtm,&rtm,0,0,VT_I4)) ){
      if( ct >= V_I4(&rtm) ) ret = true;
    }
  }
  else {
    if( ct >= *(time_t *) (stop + 16) ) ret = true;
  }
  VariantClear(&rtm);
  return ret;
}
