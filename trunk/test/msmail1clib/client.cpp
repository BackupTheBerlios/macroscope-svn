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
//------------------------------------------------------------------------------
namespace msmail {
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
    throw ksys::ExceptionSP(new ksys::Exception(code,__PRETTY_FUNCTION__));
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
    throw ksys::ExceptionSP(new ksys::Exception(r,__PRETTY_FUNCTION__));
  return r;
}
//------------------------------------------------------------------------------
void ClientFiber::auth()
{
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(client_.config_->value("max_send_size",getpagesize()));
  user = client_.config_->text("user","system");
  password = client_.config_->text("password","sha256:D7h+DiEkmuy6kSKdj9YoFurRn2Cbqoa2qGdd5kocOjE");
  encryption = client_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = client_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = client_.config_->section("compression").text(utf8::String(),"default");
  compressionType = client_.config_->section("compression").value("type","default");
  crc = client_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = client_.config_->section("compression").value("level",3);
  bool optimize = client_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = client_.config_->section("compression").value("buffer_size",getpagesize());
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
      bufferSize
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
        remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
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
          AutoPtr<OLECHAR> source(client_.name_.getOLEString());
          AutoPtr<OLECHAR> event(utf8::String("Connect").getOLEString());
          AutoPtr<OLECHAR> data(utf8::ptr2Str(this).getOLEString());
          HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
          assert( SUCCEEDED(hr) );
          AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
          client_.connectedToServer_ = remoteAddress.resolveAsync();
          client_.connected_ = true;
        }
        while( !terminated_ ){
          Message * msg;
          AutoPtr<Message> message(new Message);
          *this >> message;
          utf8::String msgId(message->id());
          {
            AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
            msg = client_.recvQueue_.find(message);
            if( msg == NULL ) client_.recvQueue_.insert(*message.ptr(NULL));
          }
          bool messageAccepted = true;
          if( msg == NULL ){
            AutoPtr<OLECHAR> source(client_.name_.getOLEString());
            AutoPtr<OLECHAR> event(utf8::String("Message").getOLEString());
            AutoPtr<OLECHAR> data(msgId.getOLEString());
            HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
            if( FAILED(hr) ){
              Exception e((hr & 0xFFFF) + errorOffset,utf8::String());
              e.writeStdError();
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
        remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
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
        remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
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
      throw ExceptionSP(new Exception(ERROR_CONNECTION_UNAVAIL + errorOffset,__PRETTY_FUNCTION__));
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( i = enumStringParts(server) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
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
    client_.data_.clear().or(tdata);
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
    AutoPtr<OLECHAR> data(utf8::int2Str(e->code()).getOLEString());
    HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
    assert( SUCCEEDED(hr) );
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Client::~Client()
{
  recvQueue_.drop();
  sendQueue_.drop();
}
//------------------------------------------------------------------------------
Client::Client() :
  pAsyncEvent_(NULL),
  config_(new ksys::InterlockedConfig<ksys::FiberInterlockedMutex>)
{
  howCloseServer(csTerminate | csShutdown | csAbort);
  fiberTimeout(0);
}
//------------------------------------------------------------------------------
void Client::open()
{
  data_.clear();
  ftime_ = 0;
  attachFiber(new ClientFiber(*this));
}
//------------------------------------------------------------------------------
void Client::close()
{
  ksock::Client::close();
  sendQueue_.clear();
  recvQueue_.clear();
}
//------------------------------------------------------------------------------
const utf8::String & Client::newMessage()
{
  Message * msg = new Message;
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
    throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
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
  if( msg == NULL ) throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
  utf8::String oldValue;
  if( msg->isValue(key) ) oldValue = msg->value(key);
  msg->value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
bool Client::sendMessage(const utf8::String id)
{
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
    attachFiber(new ClientMailSenderFiber(*this,*msg));
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
    attachFiber(new ClientMailRemoverFiber(*this,id));
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
void Client::getDB()
{
  attachFiber(new ClientDBGetterFiber(*this));
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
    throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
  AutoPtr<Message> message(new Message);
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
    throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
  return msg->removeValue(key);
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
