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
#ifndef CMAKE_BUILD
#include "msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 1
#include "msmail1clib.dir/Debug/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 2
#include "msmail1clib.dir/Release/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 3
#include "msmail1clib.dir/RelWithDebInfo/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 4
#include "msmail1clib.dir/MinSizeRel/msmail1clib.h"
#endif
#include <adicpp/adicpp.h>
#include "../msmail/msmail.h"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SerialPortFiber : public Fiber {
  friend class Client;
  public:
    virtual ~SerialPortFiber();
    SerialPortFiber(Client & client,uintptr_t serialPortNumber);
  protected:
    Client & client_;
    uintptr_t serialPortNumber_;
    AsyncFile * serial_;

    void removeFromArray();
    void fiberExecute();
  private:
    SerialPortFiber(const SerialPortFiber &);
    void operator = (const SerialPortFiber &);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class BaseClientFiber : public ksock::ClientFiber {
  public:
    virtual ~BaseClientFiber();
    BaseClientFiber();
  protected:
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);

    void main();
    virtual void connectHost(bool & online) = 0;
    virtual void auth() {}
    virtual void cycleStage0() {}
    virtual bool cycleStage1() { return false; }
    virtual void onlineStage0() {}
    virtual void onlineStage1() {}
    virtual void offlineStage0() {}
    virtual void cycleException(ExceptionSP &) {}
    virtual void onTerminate() {}
  private:
    BaseClientFiber(const BaseClientFiber &);
    void operator = (const BaseClientFiber &);

    void recvMail();
    void sendMail();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientFiber : public BaseClientFiber {
  friend class Client;
  public:
    virtual ~ClientFiber();
    ClientFiber(Client & client);
  protected:
    Client & client_;
    AutoPtr<Message> message_;
    ksock::SockAddr remoteAddress_;

    int32_t getCode2(int32_t noThrowCode0 = eOK,int32_t noThrowCode1 = eOK);

    void connectHost(bool & online);
    void auth();
    void cycleStage0();
    void onlineStage0();
    void onlineStage1();
    void cycleException(ExceptionSP &);
  private:
    ClientFiber(const ClientFiber & a) : client_(a.client_) {}
    void operator = (const ClientFiber &){}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ClientMailFiber : public BaseClientFiber {
  friend class Client;
  public:
    virtual ~ClientMailFiber();
    ClientMailFiber(Client & client);
  protected:
    Client & client_;
    FiberInterlockedMutex messageMutex_;
    FiberSemaphore semaphore_;
    class MessageControl {
      public:
        enum Operation {
          msgNone, msgSend, msgRemove
        };

        ~MessageControl(){}
        MessageControl(Message * message = NULL,Operation op = msgNone,bool async = false) :
          message_(message), operation_(op), async_(async) {}

        Message * message_;
        Operation operation_;
        bool async_;
      private:
        //MessageControl(const MessageControl &);
        //void operator = (const MessageControl &);
    };
    Vector<MessageControl> messages_;
    MessageControl * message_;

    void auth();
    void connectHost(bool & online);
    void onlineStage0();
    bool cycleStage1();
    void offlineStage0();
    void onTerminate();

    void removeMessage(MessageControl * message);
    void newMessage();
  private:
    ClientMailFiber(const ClientMailFiber &);
    void operator = (const ClientMailFiber &);
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
class MK1100ClientFiber : public ksock::ClientFiber {
  friend class Client;
  public:
    virtual ~MK1100ClientFiber();
    MK1100ClientFiber(Client & client);
  protected:
    Client & client_;
    void main();
  private:
    MK1100ClientFiber(const MK1100ClientFiber & a);
    void operator = (const MK1100ClientFiber &);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : public ksock::Client {
  friend class ClientFiber;
  friend class ClientMailFiber;
  friend class ClientDBGetterFiber;
  friend class SerialPortFiber;
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
    u_short mk1100Port_;

    HRESULT sendAsyncEvent(const utf8::String & source,const utf8::String & event,const utf8::String & data);

    utf8::String newMessage();
    HRESULT value(const utf8::String id,const utf8::String key,VARIANT * pvarRetValue) const;
    utf8::String value(const utf8::String id,const utf8::String key,const utf8::String value);
    utf8::String removeValue(const utf8::String id,const utf8::String key);

    bool sendMessage(const utf8::String id,bool async);
    bool removeMessage(const utf8::String id,bool async);
    utf8::String Client::copyMessage(const utf8::String id);
    utf8::String getReceivedMessageList() const;
    utf8::String getSendingMessageList() const;
    void getDB();
    utf8::String getDBList() const;
    utf8::String getDBGroupList() const;
    utf8::String getUserList() const;
    utf8::String getDBInGroupList(const utf8::String & group) const;

    bool installSerialPortScanner(uintptr_t serialPortNumber);
    bool removeSerialPortScanner(uintptr_t serialPortNumber);
    void removeAllSerialPortScanners();

    int32_t auth(ksock::AsyncSocket & socket);

    InterlockedMutex workFiberWait_;
    int32_t workFiberLastError_;
  protected:
    mutable FiberInterlockedMutex queueMutex_;
    Messages recvQueue_;
    AutoHashDrop<Messages> recvQueueAutoDrop_;
    Messages sendQueue_;
    AutoHashDrop<Messages> sendQueueAutoDrop_;
    Server::Data data_;
    uint64_t ftime_;
    Array<SerialPortFiber *> serialPortsFibers_;
    ClientMailFiber * clientMailFiber_;
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
      file_t handle_;
      HANDLE hEvent_;
      utf8::String name_;
      DWORD lastError_;
      bool locked_;

      ~LockedFile();
      LockedFile();
      void unlockFile();
  };
  Vector<LockedFile> files_;
  Randomizer rnd_;

  typedefEmbeddedHashKey(const wchar_t *,uint8_t,true) Function;
  typedefEmbeddedHashKeys(const wchar_t *,uint8_t,true) Functions;
  Functions functions_;
  AutoHashDrop<Functions> functionsAutoDrop_;

  typedefEmbeddedHashKey(utf8::String,VARIANTContainer,true) HashedArrayKey;
  typedefEmbeddedHashKeys(utf8::String,VARIANTContainer,true) HashedArrayKeys;
  Vector<HashedArrayKeys> hashedArrays_;

  LockedFile * findFileByName(const utf8::String & name);
  LockedFile * addFile(const utf8::String & name);

  STDMETHOD(lockFile)(IN BSTR name,IN ULONG minSleepTime,IN ULONG maxSleepTime,OUT LONG * pLastError);
  STDMETHOD(unlockFile)(IN BSTR name,OUT LONG * pLastError);
  STDMETHOD(getLastError)(IN BSTR name,OUT LONG * pLastError);
  STDMETHOD(sleep)(IN LONG sleepTime);
  STDMETHOD(sleepIn)(IN ULONG minSleepTime,IN ULONG maxSleepTime);
  STDMETHOD(textToFile)(IN BSTR name,IN BSTR text,OUT LONG * pLastError);

//  static bool isRet();

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

inline intptr_t hash(const wchar_t * a1,bool /*caseSensitive*/ = false)
{
  return ksys::HF::hash(a1,wcslen(a1) * sizeof(a1[0]));
}

inline intptr_t compareObjects(const wchar_t * a1,const wchar_t * a2,bool /*caseSensitive*/ = false)
{
//  return _wcsicoll(a1,a2);
  return wcscmp(a1,a2) ;
}

/*#include "Registry.h"

inline bool Cmsmail1c::isRet()
{
//  return false;
#if PRIVATE_RELEASE
//---------------------------------------------------------------------
  static const uint8_t stop[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    0x2c, 0xa7, 0xe6, 0x45, // 0x45E6A72C
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

//  struct tm tma;
//  memset(&tma,0,sizeof(tma));
//  tma.tm_year = 2007 - 1900;
//  tma.tm_mon = 2;
//  tma.tm_mday = 1;
//  tma.tm_hour = 13;
//  tma.tm_min = 13;
//  ct = mktime(&tma);

  time(&ct);
  if( ct >= *(time_t *) (stop + 16) ){
    ret = true;
    VARIANT rtm;
    VariantInit(&rtm);
    if( SUCCEEDED(GetRegistryKeyValue(HKEY_LOCAL_MACHINE,keyE,L"Region",&rtm)) )
      if( SUCCEEDED(VariantChangeTypeEx(&rtm,&rtm,0,0,VT_I4)) ) ret = ct >= V_I4(&rtm);
    VariantClear(&rtm);
  }
  return ret;
#else
  return false;
#endif
}
*/