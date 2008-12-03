/*-
 * Copyright (C) 2005-2007 Guram Dukashvili. All rights reserved.
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
#if CMAKE_GENERATOR_IS_VS
#if CMAKE_BUILD_TYPE == 1
#include "msmail1clib.dir/Debug/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 2
#include "msmail1clib.dir/Release/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 3
#include "msmail1clib.dir/RelWithDebInfo/msmail1clib.h"
#elif CMAKE_BUILD_TYPE == 4
#include "msmail1clib.dir/MinSizeRel/msmail1clib.h"
#endif
#else
#include "msmail1clib.h"
#endif
#include <adicpp/adicpp.h>
#include "../msmail/msmail.h"
//------------------------------------------------------------------------------
class Cmsmail1c;
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SerialPortFiber : public Fiber {
  friend class Client;
  public:
    virtual ~SerialPortFiber();
    SerialPortFiber() {}
    SerialPortFiber(Client * client,uintptr_t serialPortNumber);
  protected:
    Client * client_;
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
    ClientFiber() {}
    ClientFiber(Client * client,bool terminateAfterCheck = false,bool waitForMail = true,bool onlyNewMail = true);
  protected:
    Client * client_;
    AutoPtr<Message> message_;
    ksock::SockAddr remoteAddress_;
    bool terminateAfterCheck_;
    bool waitForMail_;
    bool onlyNewMail_;

    int32_t getCode2(int32_t noThrowCode0 = eOK,int32_t noThrowCode1 = eOK);

    void connectHost(bool & online);
    void auth();
    void cycleStage0();
    void onlineStage0();
    void onlineStage1();
    void cycleException(ExceptionSP & e);
    void onTerminate();
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
    ClientMailFiber() : messagesAutoDrop_(messages_) {}
    ClientMailFiber(Client * client);
  protected:
    Client * client_;
    FiberWriteLock messageReadWriteLock_;
    FiberSemaphore semaphore_;
    class MessageControl {
      public:
        enum Operation {
          msgNone, msgSend, msgRemove
        };

        virtual ~MessageControl(){}
        MessageControl(Message * message = NULL,Operation op = msgNone,bool async = false) :
          message_(message), operation_(op), async_(async) {}

        mutable EmbeddedListNode<MessageControl> node_;
        static EmbeddedListNode<MessageControl> & node(const MessageControl & object){
          return object.node_;
        }
        static MessageControl & nodeObject(const EmbeddedListNode<MessageControl> & node,MessageControl * p = NULL){
          return node.object(p->node_);
        }

        Message * message_;
        Operation operation_;
        bool async_;
      private:
        //MessageControl(const MessageControl &);
        //void operator = (const MessageControl &);
    };
    typedef EmbeddedList<
      MessageControl,
      MessageControl::node,
      MessageControl::nodeObject
    > Messages;
    Messages messages_;
    AutoDrop<Messages> messagesAutoDrop_;
    MessageControl * message_;

    void auth();
    void connectHost(bool & online);
    void cycleException(ExceptionSP & e);
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
    ClientDBGetterFiber() {}
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
class MK1100TCPServer;
//------------------------------------------------------------------------------
class MK1100ClientFiber : public ksock::ServerFiber {
  friend class Cmsmail1c;
  public:
    virtual ~MK1100ClientFiber();
    MK1100ClientFiber(Client * client = NULL,MK1100TCPServer * server = NULL);
  protected:
    Client * client_;
    MK1100TCPServer * server_;
    FiberSemaphore sem_;
    utf8::String data_;

    void fiberBreakExecution();
    void mainHelper();
    void main();
    utf8::String readString();
    MK1100ClientFiber & writeString(const utf8::String & s);
  private:
    MK1100ClientFiber(const MK1100ClientFiber & a);
    void operator = (const MK1100ClientFiber &);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MK1100TCPServer : public ksock::Server {
  friend class Cmsmail1c;
  friend class MK1100ClientFiber;
  public:
    virtual ~MK1100TCPServer();
    MK1100TCPServer(Client * client = NULL);

    void open();
  protected:
    Client * client_;
    FiberWriteLock fibersReadWriteLock_;
    Array<MK1100ClientFiber *> fibers_;

    Fiber * newFiber();
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : public ksock::Client {
  friend class Cmsmail1c;
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
    mutable FiberWriteLock connectedReadWriteLock_;
    utf8::String connectedToServer_;
    bool connected_;
    bool asyncMessagesReceiving_;
    u_short mk1100Port_;
    AutoPtr<MK1100TCPServer> mk1100TCPServer_;

    HRESULT sendAsyncEvent(const utf8::String & source,const utf8::String & event,const utf8::String & data);

    utf8::String newMessage();
    HRESULT value(const utf8::String id,const utf8::String key,VARIANT * pvarRetValue) const;
    utf8::String value(const utf8::String id,const utf8::String key,const utf8::String value);
    utf8::String removeValue(const utf8::String id,const utf8::String key);

    bool attachFileToMessage(const utf8::String id,const utf8::String key,const utf8::String fileName);
    bool saveMessageAttachmentToFile(const utf8::String id,const utf8::String key,const utf8::String fileName);

    bool receiveMessages(bool onlyNewMail);
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

    Client & readConfig(const utf8::String & configFile,const utf8::String & logFile);

    WriteLock workFiberWait_;
    int32_t workFiberLastError_;
  protected:
    mutable FiberWriteLock queueReadWriteLock_;
    Messages recvQueue_;
    AutoDrop<Messages> recvQueueAutoDrop_;
    Messages sendQueue_;
    AutoDrop<Messages> sendQueueAutoDrop_;
    Server::Data data_;
    uint64_t ftime_;
    Array<SerialPortFiber *> serialPortsFibers_;
    ClientMailFiber * clientMailFiber_;
  private:
    Client(const Client &);
    void operator = (const Client &) { assert( 0 ); }
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
  virtual ~Cmsmail1c() throw();
  Cmsmail1c();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  DECLARE_REGISTRY_RESOURCEID(IDR_MSMAIL1C)

  BEGIN_COM_MAP(Cmsmail1c)
    COM_INTERFACE_ENTRY(Imsmail1c)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IInitDone)
    COM_INTERFACE_ENTRY(ILanguageExtender)
  END_COM_MAP()

  DECLARE_PROTECT_FINAL_CONSTRUCT()
#endif

  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

public:
  class msmail1c {
    public:
      virtual ~msmail1c();
      msmail1c();

      adicpp::AutoInitializer initializer_;

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
          AsyncFile file_;
          utf8::String name_;
          DWORD lastError_;
          bool locked_;

          virtual ~LockedFile();
          LockedFile();
          void unlockFile();
      };
      Vector<LockedFile> files_;
      Randomizer rnd_;

      typedefEmbeddedHashKey(const wchar_t *,uint8_t,true) Function;
      typedefEmbeddedHashKeys(const wchar_t *,uint8_t,true) Functions;
      Functions functions_;
      AutoDrop<Functions> functionsAutoDrop_;

      typedefEmbeddedHashKey(utf8::String,VARIANTContainer,true) HashedArrayKey;
      typedefEmbeddedHashKeys(utf8::String,VARIANTContainer,true) HashedArrayKeys;
      Vector<HashedArrayKeys> hashedArrays_;

      LockedFile * findFileByName(const utf8::String & name);
      LockedFile * addFile(const utf8::String & name);
      // Решение проблемы Сервер занят
      class ServerBusyThread : public Thread {
        public:
          virtual ~ServerBusyThread() {}
          ServerBusyThread(msmail1c * parent = NULL) : parent_(parent) {}
        protected:
        private:
          msmail1c * parent_;
          Semaphore sem_;
          Array<uint8_t> text_;
          Array<HWND> windows_;
          uintptr_t count_;
          union {
            const char * textToFindA_;
            const wchar_t * textToFindW_;
          };
          static BOOL CALLBACK enumWindowsProc(HWND hwnd,LPARAM lParam);
          void threadBeforeWait();
          void threadExecute();

          ServerBusyThread(const ServerBusyThread &);
          void operator = (const ServerBusyThread &);
      };
      AutoPtr<ServerBusyThread> serverBusyThread_;
  };

  AutoPtr<msmail1c> msmail1c_;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
#endif
  static void * oldBKENDGetProcAddress_;
  static void * oldDBENG32LockFile_;
  static void * oldDBENG32FlushFileBuffers_;
  static void * oldMSVCRTLockFile_;
  static void * oldMSVCR71LockFile_;
  static void * oldMFC42LockFile_;
  static uint64_t lastLockFileCallTime_;
  union ImportedEntry {
    FARPROC (WINAPI * f_)(HMODULE,LPCSTR);
    void * p_;
  };
  static ImportedEntry oldSEVENGetProcAddress_;
  static FARPROC WINAPI reparedGetProcAddress(HMODULE hModule,LPCSTR lpProcName);

  uint8_t lockFileJmpCodeSafe_[7];
  uint8_t unLockFileJmpCodeSafe_[7];
  uint8_t flushFileBuffersJmpCodeSafe_[7];

  static BOOL WINAPI repairedLockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh
  );
  static BOOL WINAPI repairedUnlockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh
  );
  static BOOL WINAPI repairedFlushFileBuffers(HANDLE hFile);
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
OBJECT_ENTRY_AUTO(__uuidof(msmail1c), Cmsmail1c)
#endif

inline intptr_t hash(const wchar_t * a1,bool /*caseSensitive*/ = false)
{
  return ksys::HF::hash(a1,wcslen(a1) * sizeof(a1[0]));
}

inline intptr_t compareObjects(const wchar_t * a1,const wchar_t * a2,bool /*caseSensitive*/ = false)
{
//  return _wcsicoll(a1,a2);
  return wcscmp(a1,a2) ;
}
//------------------------------------------------------------------------------
