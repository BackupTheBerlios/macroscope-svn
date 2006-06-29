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
  intptr_t i, c;
  client_.config_->fileName(client_.configFile_).parse();
  while( !terminated_ ){
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
        *this << uint8_t(cmRegisterClient) <<
          UserInfo(client_.user_) << KeyInfo(client_.key_);
        uint64_t u = enumStringParts(client_.groups_);
        *this << u;
        for( i = intptr_t(u - 1); i >= 0 && !terminated_; i-- )
          *this << GroupInfo(stringPartByNo(client_.groups_,i));
        if( terminated_ ) break;
        getCode();
        *this << uint8_t(cmRecvMail) << client_.user_ << client_.key_ << bool(true) << bool(true);
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
          AutoPtr<Message> message(msg = new Message);
          *this >> message;
          {
            AutoLock<FiberInterlockedMutex> lock(client_.recvQueueMutex_);
            i = client_.recvQueue_.bSearch(message,c);
            if( c != 0 ) client_.recvQueue_.safeInsert(i + (c > 0),message.ptr(NULL));
          }
          bool messageAccepted = true;
          if( c != 0 ){
            AutoPtr<OLECHAR> source(client_.name_.getOLEString());
            AutoPtr<OLECHAR> event(utf8::String("Message").getOLEString());
            AutoPtr<OLECHAR> data(msg->id().getOLEString());
            HRESULT hr = client_.pAsyncEvent_->ExternalEvent(source.ptr(NULL),event.ptr(NULL),data.ptr(NULL));
            if( FAILED(hr) ){
              Exception e((hr & 0xFFFF) + errorOffset,utf8::String());
              e.writeStdError();
              messageAccepted = false;
            }
          }
          *this << messageAccepted;
          getCode2(eLastMessage);
          client_.config_->parse();
          stdErr.rotationThreshold(client_.config_->value("debug_file_rotate_threshold",1024 * 1024));
          stdErr.rotatedFileCount(client_.config_->value("debug_file_rotate_count",10));
          stdErr.setDebugLevels(client_.config_->value("debug_levels","+0,+1,+2,+3"));
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
    for( i = enumStringParts(client_.mailServer_) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveAsync(stringPartByNo(client_.mailServer_,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(client_.mailServer_,i) <<
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
    for( i = enumStringParts(client_.mailServer_) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveAsync(stringPartByNo(client_.mailServer_,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(client_.mailServer_,i) <<
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
    for( i = enumStringParts(client_.mailServer_) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveAsync(stringPartByNo(client_.mailServer_,i),defaultPort);
        connect(remoteAddress);
        auth();
        i = -1;
      }
      catch( ExceptionSP & e ){
        stdErr.debug(2,utf8::String::Stream() <<
          "Unable to connect. Host " << stringPartByNo(client_.mailServer_,i) <<
          " unreachable.\n"
        );
        if( terminated_ || i == 0 ) throw; else e->writeStdError();
      }
    }
    *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
    getCode();
    *this << uint8_t(cmGetDB) << client_.ftime_;
    client_.data_.recvDatabase(*this);
    *this >> client_.ftime_;
    getCode();
    *this << uint8_t(cmQuit);
    getCode();
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
  AutoPtr<Message> message(new Message);
  intptr_t c, i = sendQueue_.bSearch(message->id(),c);
  sendQueue_.insert(i + (c > 0),message.ptr());
  return message.ptr(NULL)->id();
}
//------------------------------------------------------------------------------
const utf8::String & Client::value(const utf8::String id,const utf8::String key) const
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  const Vector<Message> * queue = &sendQueue_;
  intptr_t i = queue->bSearch(Message(id));
  if( i < 0 ){
    queue = &recvQueue_;
    i = queue->bSearch(Message(id));
  }
  if( i < 0 ) throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
  return (*queue)[i].value(key);
}
//------------------------------------------------------------------------------
utf8::String Client::value(const utf8::String id,const utf8::String key,const utf8::String value)
{
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  Vector<Message> * queue = &sendQueue_;
  intptr_t i = queue->bSearch(Message(id));
  if( i < 0 ){
    queue = &recvQueue_;
    i = queue->bSearch(id);
  }
  if( i < 0 ) throw ExceptionSP(new Exception(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__));
  utf8::String oldValue;
  if( (*queue)[i].isValue(key) ) oldValue = ((*queue)[i].value(key));
  (*queue)[i].value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
bool Client::sendMessage(const utf8::String id)
{
  intptr_t i = sendQueue_.bSearch(id);
  if( i < 0 ) return false;
  workFiberLastError_ = 0;
  sendQueue_[i].value("#Sender",user_ + "@" + key_);
  sendQueue_[i].value("#Sender.Sended",getTimeString(gettimeofday()));
  sendQueue_[i].value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
  sendQueue_[i].value("#Sender.Process.StartTime",getTimeString(getProcessStartTime()));
  sendQueue_[i].value("#Sender.Host",ksock::SockAddr::gethostname());
  workFiberWait_.acquire();
  try {
    attachFiber(new ClientMailSenderFiber(*this,sendQueue_[i]));
  }
  catch( ... ){
    workFiberWait_.release();
    throw;
  }
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  if( workFiberLastError_ == 0 ) sendQueue_.remove(i);
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
bool Client::removeMessage(const utf8::String id)
{
  Vector<Message> * queue = &sendQueue_;
  intptr_t i = sendQueue_.bSearch(Message(id));
  if( i >= 0 ){
    sendQueue_.remove(i);
    return true;
  }
  {
    AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
    i = recvQueue_.bSearch(id);
    if( i < 0 ) return false;
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
    i = recvQueue_.bSearch(id);
    assert( i >= 0 );
    recvQueue_.remove(i);
  }
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
utf8::String Client::getReceivedMessageList() const
{
  utf8::String list;
  AutoLock<FiberInterlockedMutex> lock(recvQueueMutex_);
  for( intptr_t i = recvQueue_.count() - 1; i >= 0; i-- ){
    list += "\"";
    list += recvQueue_[i].id();
    list += "\"";
    if( i > 0 ) list += ",";
  }
  return list;
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
  return data_.getKeyGroupList(groups_,true);
}
//------------------------------------------------------------------------------
utf8::String Client::getUserList() const
{
  return data_.getUserList(true);
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
