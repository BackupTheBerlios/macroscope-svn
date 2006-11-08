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
#include "stdafx.h"
#include "msmail1cImpl.h"
#include "../msmail/info.cpp"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SerialPortFiber::~SerialPortFiber()
{
}
//------------------------------------------------------------------------------
SerialPortFiber::SerialPortFiber(Client & client,uintptr_t serialPortNumber) :
  client_(client), serialPortNumber_(serialPortNumber), serial_(NULL)
{
}
//------------------------------------------------------------------------------
void SerialPortFiber::removeFromArray()
{
  AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
  intptr_t i = client_.serialPortsFibers_.bSearch(this);
  assert( i >= 0 );
  client_.serialPortsFibers_.remove(i);
}
//------------------------------------------------------------------------------
void SerialPortFiber::fiberExecute()
{
  int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD evtMask;
#else
  uint32_t evtMask;
#endif

  AsyncFile serial;
  serial_ = &serial;
  try {
    serial.detachOnClose(false).readOnly(true).exclusive(true).fileName(
      "COM" + utf8::int2Str(serialPortNumber_) + ":"
    ).open();
#if defined(__WIN32__) || defined(__WIN64__)
    if( SetupComm(serial.descriptor(),1600,1600) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    COMMTIMEOUTS cto;
    if( GetCommTimeouts(serial.descriptor(),&cto) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    memset(&cto,0,sizeof(cto));
    cto.ReadIntervalTimeout = MAXDWORD;
    /*cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 0;
    cto.WriteTotalTimeoutMultiplier = MAXDWORD;
    cto.WriteTotalTimeoutConstant = MAXDWORD - 1;*/
    if( SetCommTimeouts(serial.descriptor(),&cto) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( GetCommMask(serial.descriptor(),&evtMask) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( SetCommMask(serial.descriptor(),evtMask | EV_RXCHAR/* | EV_CTS | EV_DSR | EV_ERR | EV_RLSD*/) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#endif
    AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
    intptr_t c, i = client_.serialPortsFibers_.bSearch(this,c);
    assert( c != 0 );
    client_.serialPortsFibers_.insert(i + (c > 0),this);
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_.workFiberLastError_ = e->code();
  }
  try {
    client_.workFiberWait_.release();
    if( serial.isOpen() ){
      Array<uint8_t> b;
      uintptr_t pos = 0, i, j;
      b.resize(8);
      while( !terminated_ ){
        if( pos >= b.count() ) b.resize((b.count() << 1) + (b.count() == 0));
        memset(b.ptr() + pos,0,b.count() - pos);
        evtMask = serial.waitCommEvent();
        int64_t r = serial.read(b.ptr() + pos,b.count() - pos);
        if( r < 0 ) break;
        pos += (uintptr_t) r;
        for( i = 0; i < pos; ){
          while( i < pos ){
            if( isascii(b[i]) && !iscntrl(b[i]) ) break;
            i++;
          }
          j = i;
          bool codeBar = false;
          while( i < pos ){
            codeBar = !isascii(b[i]) || iscntrl(b[i]);
            if( codeBar ) break;
            i++;
          }
          if( codeBar ){
            BSTR source = NULL, event = NULL, data = NULL;
            try {
              source = client_.name_.getOLEString();
              event = utf8::String("BarCodeValue_COM" + utf8::int2Str(serialPortNumber_)).getOLEString();
              data = utf8::String((char *) b.ptr() + j,i - j).getOLEString();
            }
            catch( ... ){
              SysFreeString(data);
              SysFreeString(event);
              SysFreeString(source);
              throw;
            }
            HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source,event,data);
            if( FAILED(hr) ){
              newObject<Exception>(
                HRESULT_CODE(hr) + errorOffset,
                utf8::String(__PRETTY_FUNCTION__) + " line " +
                utf8::int2Str(__LINE__) + ", pAsyncEvent_->ExternalEvent(" +
                client_.name_ + "," +
                "BarCodeValue_COM" + utf8::int2Str(serialPortNumber_) + "," +
                utf8::String((char *) b.ptr() + j,i - j) + ") failed"
              )->writeStdError();
            }
            memcpy(b.ptr(),b.ptr() + i,pos - i);
            //memset(b.ptr() + pos - i,0,b.count() + pos - i);
            pos -= i;
            i = 0;
          }
        }
      }
      removeFromArray();
      client_.workFiberLastError_ = 0;
      client_.workFiberWait_.release();
    }
  }
  catch( ... ){
    removeFromArray();
    if( terminated_ ){
      client_.workFiberLastError_ = 0;
      client_.workFiberWait_.release();
    }
    else {
      throw;
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientFiber::~ClientFiber()
{
}
//------------------------------------------------------------------------------
ClientFiber::ClientFiber(Client & client) : client_(client)
{
}
//------------------------------------------------------------------------------
void ClientFiber::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void ClientFiber::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
int32_t ClientFiber::getCode2(int32_t noThrowCode0,int32_t noThrowCode1)
{
  int32_t r;
  *this >> r;
  if( r != eOK && r != noThrowCode0 && r != noThrowCode1 )
    newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r;
}
//------------------------------------------------------------------------------
void ClientFiber::auth()
{
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(client_.config_->value("max_send_size",getpagesize()));
  user = client_.config_->text("user","system");
  password = client_.config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  encryption = client_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = client_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = client_.config_->section("compression").text(utf8::String(),"default");
  compressionType = client_.config_->section("compression").value("type","default");
  crc = client_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = client_.config_->section("compression").value("level",3);
  bool optimize = client_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = client_.config_->section("compression").value("buffer_size",getpagesize());
  bool noAuth = client_.config_->value("noauth",false);
  checkCode(
    clientAuth(
      user,
      password,
      encryption,
      encryptionThreshold,
      compression,
      compressionType,
      crc,
      compressionLevel,
      optimize,
      bufferSize,
      noAuth
    )
  );
}
//------------------------------------------------------------------------------
void ClientFiber::main()
{
  intptr_t i;
  client_.config_->fileName(client_.configFile_);
  while( !terminated_ ){
    client_.config_->parse();
    stdErr.rotationThreshold(client_.config_->value("debug_file_rotate_threshold",1024 * 1024));
    stdErr.rotatedFileCount(client_.config_->value("debug_file_rotate_count",10));
    stdErr.setDebugLevels(client_.config_->value("debug_levels","+0,+1,+2,+3"));
    checkMachineBinding(client_.config_->value("machine_key"));
    {
      AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
      client_.connected_ = false;
      client_.connectedToServer_.resize(0);
    }
    try {
      bool connected = false;
      ksock::SockAddr remoteAddress;
      utf8::String server(client_.config_->value("server",client_.mailServer_));
      for( i = enumStringParts(server) - 1; i >= 0 && !terminated_ && !client_.connected_; i-- ){
        remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
        try {
          connect(remoteAddress);
          try {
            auth();
            connected = true;
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
            stdErr.debug(3,
              utf8::String::Stream() << "Authentification to host " <<
              stringPartByNo(server,i) << " failed.\n"
            );
          }
        }
        catch( ExceptionSP & e ){
          e->writeStdError();
          stdErr.debug(3,
            utf8::String::Stream() << "Unable to connect. Host " <<
            stringPartByNo(server,i) <<
            " unreachable.\n"
          );
        }
      }
      if( connected ){
        *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
        getCode();
        utf8::String key(client_.config_->value("key",client_.key_));
        *this << uint8_t(cmRegisterClient) <<
          UserInfo(client_.user_) <<
          KeyInfo(key);
        utf8::String groups(client_.config_->value("groups",client_.groups_));
        uint64_t u = enumStringParts(groups);
        *this << u;
        for( i = intptr_t(u - 1); i >= 0 && !terminated_; i-- )
          *this << GroupInfo(stringPartByNo(groups,i));
        if( terminated_ ) break;
        getCode();
        *this << uint8_t(cmRecvMail) << client_.user_ << key << bool(true) << bool(true);
        getCode();
        {
          BSTR source = NULL, event = NULL, data = NULL;
          try {
            source = client_.name_.getOLEString();
            event = utf8::String("Connect").getOLEString();
            data = utf8::ptr2Str(this).getOLEString();
          }
          catch( ... ){
            SysFreeString(data);
            SysFreeString(event);
            SysFreeString(source);
            throw;
          }
          HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source,event,data);
          if( FAILED(hr) ){
            newObject<Exception>(
              HRESULT_CODE(hr) + errorOffset,
              utf8::String(__PRETTY_FUNCTION__) + " line " +
              utf8::int2Str(__LINE__) + ", pAsyncEvent_->ExternalEvent(" +
              client_.name_ + ",Connect," +
              utf8::ptr2Str(this) + ") failed"
            )->writeStdError();
          }
          AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
          client_.connectedToServer_ = remoteAddress.resolve();
          client_.connected_ = true;
        }
        while( !terminated_ ){
          Message * msg;
          AutoPtr<Message> message(newObject<Message>());
          *this >> message;
          utf8::String msgId(message->id());
          {
            AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
            msg = client_.recvQueue_.find(message);
            if( msg == NULL ) client_.recvQueue_.insert(*message.ptr(NULL));
          }
          bool messageAccepted = true;
          if( msg == NULL ){
            BSTR source = NULL, event = NULL, data = NULL;
            try {
              source = client_.name_.getOLEString();
              event = utf8::String("Message").getOLEString();
              data = msgId.getOLEString();
            }
            catch( ... ){
              SysFreeString(data);
              SysFreeString(event);
              SysFreeString(source);
              throw;
            }
            HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source,event,data);
            if( FAILED(hr) ){
              newObject<Exception>(
                HRESULT_CODE(hr) + errorOffset,
                utf8::String(__PRETTY_FUNCTION__) + " line " +
                utf8::int2Str(__LINE__) + ", pAsyncEvent_->ExternalEvent(" +
                client_.name_ + ",Message," +
                msgId + ") failed"
              )->writeStdError();
              messageAccepted = false;
              AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
              Message * msg = client_.recvQueue_.find(msgId);
              if( msg != NULL ) client_.recvQueue_.drop(*msg);
            }
          }
          *this << messageAccepted;
          getCode2(eLastMessage);
        }
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
    shutdown();
    close();
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientMailSenderFiber::~ClientMailSenderFiber()
{
}
//------------------------------------------------------------------------------
ClientMailSenderFiber::ClientMailSenderFiber(Client & client,Message & message) :
  ClientFiber(client), message_(message)
{
}
//------------------------------------------------------------------------------
void ClientMailSenderFiber::main()
{
  union {
    intptr_t i;
    uintptr_t u;
  };
  try {
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( i = enumStringParts(server) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(server,i) <<
          " unreachable.\n"
        );
        if( terminated_ || i == 0 ) throw; else e->writeStdError();
      }
    }
    *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
    getCode();
    *this << uint8_t(cmSendMail) << message_.id() << false /* no rest flag */ << message_;
    getCode();
    *this << uint8_t(cmQuit);
    getCode();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_.workFiberLastError_ = e->code();
  }
  client_.workFiberWait_.release();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientMailRemoverFiber::~ClientMailRemoverFiber()
{
}
//------------------------------------------------------------------------------
ClientMailRemoverFiber::ClientMailRemoverFiber(Client & client,const utf8::String & id) :
  ClientFiber(client), id_(id)
{
}
//------------------------------------------------------------------------------
void ClientMailRemoverFiber::main()
{
  union {
    intptr_t i;
    uintptr_t u;
  };
  try {
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( i = enumStringParts(server) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(server,i) <<
          " unreachable.\n"
        );
        if( terminated_ || i == 0 ) throw; else e->writeStdError();
      }
    }
    *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
    getCode();
    *this << uint8_t(cmRemoveMail) << client_.user_ << id_;
    getCode();
    *this << uint8_t(cmQuit);
    getCode();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_.workFiberLastError_ = e->code();
  }
  client_.workFiberWait_.release();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientDBGetterFiber::~ClientDBGetterFiber()
{
}
//------------------------------------------------------------------------------
ClientDBGetterFiber::ClientDBGetterFiber(Client & client) : ClientFiber(client)
{
}
//------------------------------------------------------------------------------
void ClientDBGetterFiber::main()
{
  union {
    intptr_t i;
    uintptr_t u;
  };
  try {
    bool registered;
    {
      AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
      registered = client_.connected_;
    }
    if( !registered )
      newObject<Exception>(ERROR_CONNECTION_UNAVAIL + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( i = enumStringParts(server) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(server,i) <<
          " unreachable.\n"
        );
        if( terminated_ || i == 0 ) throw; else e->writeStdError();
      }
    }
    Server::Data tdata;
    *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
    getCode();
    *this << uint8_t(cmGetDB);
    tdata.recvDatabase(*this);
    getCode();
    *this << uint8_t(cmQuit);
    getCode();
    client_.data_.clear().ore(tdata);
    AutoPtr<OLECHAR> source(client_.name_.getOLEString());
    AutoPtr<OLECHAR> event(utf8::String("GetDB").getOLEString());
    bool isEmpty = 
      client_.data_.getUserList().strlen() == 0 &&
      client_.data_.getKeyList().strlen() == 0
    ;
    AutoPtr<OLECHAR> data(utf8::String(isEmpty ? "" : "DATA").getOLEString());
    HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
    assert( SUCCEEDED(hr) );
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    AutoPtr<OLECHAR> source(client_.name_.getOLEString());
    AutoPtr<OLECHAR> event(utf8::String("GetDB").getOLEString());
    AutoPtr<OLECHAR> data(utf8::int2Str(e->code() - (e->code() >= errorOffset) * errorOffset).getOLEString());
    HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
    assert( SUCCEEDED(hr) );
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MK1100ClientFiber::~MK1100ClientFiber()
{
}
//------------------------------------------------------------------------------
MK1100ClientFiber::MK1100ClientFiber(Client & client) : client_(client)
{
}
//------------------------------------------------------------------------------
void MK1100ClientFiber::main()
{
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Client::~Client()
{
}
//------------------------------------------------------------------------------
Client::Client() :
  pAsyncEvent_(NULL),
  config_(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()),
  recvQueueAutoDrop_(recvQueue_),
  sendQueueAutoDrop_(sendQueue_),
  mk1100Port_(0)
{
  howCloseServer(csTerminate | csShutdown | csAbort);
  fiberTimeout(0);
}
//------------------------------------------------------------------------------
void Client::open()
{
  data_.clear();
  ftime_ = 0;
  attachFiber(newObjectV<ClientFiber>(*this));
  if( mk1100Port_ != 0 )
    attachFiber(newObjectV<MK1100ClientFiber>(*this));
}
//------------------------------------------------------------------------------
void Client::close()
{
  removeAllSerialPortScanners();
  ksock::Client::close();
  sendQueue_.drop();
  recvQueue_.drop();
}
//------------------------------------------------------------------------------
const utf8::String & Client::newMessage()
{
  Message * msg = newObject<Message>();
  sendQueue_.insert(*msg);
  return msg->id();
}
//------------------------------------------------------------------------------
HRESULT Client::value(const utf8::String id,const utf8::String key,VARIANT * pvarRetValue) const
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  const Messages * queue = &sendQueue_;
  const Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL )
    newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  HRESULT hr = S_OK;
  if( msg->isValue(key) ){
    V_BSTR(pvarRetValue) = msg->value(key).getOLEString();
    V_VT(pvarRetValue) = VT_BSTR;
  }
  else {
    hr = VariantChangeTypeEx(pvarRetValue,pvarRetValue,0,0,VT_EMPTY);
  }
  return hr;
}
//------------------------------------------------------------------------------
utf8::String Client::value(const utf8::String id,const utf8::String key,const utf8::String value)
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  Messages * queue = &sendQueue_;
  Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL ) newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  utf8::String oldValue;
  if( msg->isValue(key) ) oldValue = msg->value(key);
  msg->value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
bool Client::sendMessage(const utf8::String id)
{
  checkMachineBinding(config_->value("machine_key"));
  Message * msg = sendQueue_.find(id);
  if( msg == NULL ) return false;
  workFiberLastError_ = 0;
  msg->value("#Sender",user_ + "@" + config_->value("key",key_));
  msg->value("#Sender.Sended",getTimeString(gettimeofday()));
  msg->value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
  msg->value("#Sender.Process.StartTime",getTimeString(getProcessStartTime()));
  msg->value("#Sender.Host",ksock::SockAddr::gethostname());
  workFiberWait_.acquire();
  try {
    attachFiber(newObjectV<ClientMailSenderFiber>(*this,*msg));
  }
  catch( ... ){
    workFiberWait_.release();
    throw;
  }
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  if( workFiberLastError_ == 0 ) sendQueue_.drop(*msg);
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
bool Client::removeMessage(const utf8::String id)
{
  Messages * queue = &sendQueue_;
  Message * msg = sendQueue_.find(id);
  if( msg != NULL ){
    sendQueue_.drop(*msg);
    return true;
  }
  {
    AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
    msg = recvQueue_.find(id);
    if( msg == NULL ) return false;
  }
  workFiberWait_.acquire();
  workFiberLastError_ = 0;
  try {
    attachFiber(newObjectV<ClientMailRemoverFiber>(*this,id));
  }
  catch( ... ){
    workFiberWait_.release();
    throw;
  }
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  if( workFiberLastError_ == 0 ){
    AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
    msg = recvQueue_.find(id);
    assert( msg != NULL );
    recvQueue_.drop(*msg);
  }
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
utf8::String Client::getReceivedMessageList() const
{
  utf8::String slist;
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  Array<Message *> list;
  recvQueue_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    slist += "\"";
    slist += list[i]->id();
    slist += "\"";
    if( i > 0 ) slist += ",";
  }
  return slist;
}
//------------------------------------------------------------------------------
utf8::String Client::getSendingMessageList() const
{
  utf8::String slist;
  Array<Message *> list;
  sendQueue_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    slist += "\"";
    slist += list[i]->id();
    slist += "\"";
    if( i > 0 ) slist += ",";
  }
  return slist;
}
//------------------------------------------------------------------------------
void Client::getDB()
{
  attachFiber(newObjectV<ClientDBGetterFiber>(*this));
}
//------------------------------------------------------------------------------
utf8::String Client::getDBList() const
{
  return data_.getKeyList(true);
}
//------------------------------------------------------------------------------
utf8::String Client::getDBGroupList() const
{
  utf8::String groups(config_->value("groups",groups_));
  return data_.getKeyGroupList(groups,true);
}
//------------------------------------------------------------------------------
utf8::String Client::getUserList() const
{
  return data_.getUserList(true);
}
//------------------------------------------------------------------------------
utf8::String Client::getDBInGroupList(const utf8::String & group) const
{
  return data_.getKeyInGroupList(group,true);
}
//------------------------------------------------------------------------------
utf8::String Client::copyMessage(const utf8::String id)
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  const Messages * queue = &sendQueue_;
  Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL )
    newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  AutoPtr<Message> message(newObject<Message>());
  utf8::String newId(message->id());
  *message = *msg;
  message->removeValueByLeft("#Relay");
  message->id(newId);
  sendQueue_.insert(*message.ptr(NULL));
  return newId;
}
//------------------------------------------------------------------------------
utf8::String Client::removeValue(const utf8::String id,const utf8::String key)
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  Messages * queue = &sendQueue_;
  Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL )
    newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return msg->removeValue(key);
}
//------------------------------------------------------------------------------
bool Client::installSerialPortScanner(uintptr_t serialPortNumber)
{
  bool installed = false;
  {
    AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
    for( intptr_t i = serialPortsFibers_.count() - 1; i >= 0; i-- )
      if( serialPortsFibers_[i]->serialPortNumber_ == serialPortNumber ){
        installed = true;
        break;
      }
  }
  workFiberLastError_ = 0;
  if( !installed ){
    workFiberWait_.acquire();
    try {
      attachFiber(newObjectV<SerialPortFiber>(*this,serialPortNumber));
    }
    catch( ... ){
      workFiberWait_.release();
      throw;
    }
    AutoLock<InterlockedMutex> lock(workFiberWait_);
  }
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
bool Client::removeSerialPortScanner(uintptr_t serialPortNumber)
{
  intptr_t i;
  {
    AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
    for( i = serialPortsFibers_.count() - 1; i >= 0; i-- )
      if( serialPortsFibers_[i]->serialPortNumber_ == serialPortNumber ){
        serialPortsFibers_[i]->terminate();
        serialPortsFibers_[i]->serial_->close();
        break;
      }
  }
  if( i < 0 ) return false;
  workFiberLastError_ = 0;
  workFiberWait_.acquire();
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
void Client::removeAllSerialPortScanners()
{
  for(;;){
    intptr_t i;
    {
      AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
      i = serialPortsFibers_.count() - 1;
      if( i >= 0 ){
        serialPortsFibers_[i]->terminate();
        serialPortsFibers_[i]->serial_->close();
      }
    }
    if( i < 0 ) break;
    workFiberLastError_ = 0;
    workFiberWait_.acquire();
    AutoLock<InterlockedMutex> lock(workFiberWait_);
  }
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
