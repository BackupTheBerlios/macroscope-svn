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
  stdErr.fileName(client_.logFile_);
  client_.config_->fileName(client_.configFile_).parse();
  bool connected = false;
  while( !terminated_ ){
    try {
      while( !terminated_ && !connected ){
        for( i = enumStringParts(client_.mailServer_) - 1; i >= 0 && !terminated_ && !connected; i-- ){
          ksock::SockAddr remoteAddress;
          remoteAddress.resolveAsync(stringPartByNo(client_.mailServer_,i),defaultPort);
          connect(remoteAddress);
          auth();
          connected = true;
        }
      }
      *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
      getCode();
      *this << uint8_t(cmRegisterUser) << UserInfo(client_.user_);
      getCode();
      *this << uint8_t(cmRegisterKey) << KeyInfo(client_.key_);
      getCode();
      for( i = enumStringParts(client_.groups_) - 1; i >= 0 && !terminated_; i-- ){
        *this << uint8_t(cmRegisterGroup) << GroupInfo(stringPartByNo(client_.groups_,i));
        getCode();
      }
      *this << uint8_t(cmRegisterUser2KeyLink) <<
        UserInfo(client_.user_) << KeyInfo(client_.key_);
      getCode();
      for( i = enumStringParts(client_.groups_) - 1; i >= 0 && !terminated_; i-- ){
        *this << uint8_t(cmRegisterKey2GroupLink) <<
          KeyInfo(client_.key_) << GroupInfo(stringPartByNo(client_.groups_,i));
        getCode();
      }
      *this << uint8_t(cmRecvMail) << client_.user_ << client_.key_;
      getCode();
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      shutdown();
      close();
      connected = false;
    }
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
    for( i = enumStringParts(client_.mailServer_) - 1; i >= 0 && !terminated_; i-- ){
      ksock::SockAddr remoteAddress;
      remoteAddress.resolveAsync(stringPartByNo(client_.mailServer_,i),defaultPort);
      try {
        connect(remoteAddress);
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
      }
      auth();
      break;
    }
    *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
    getCode();
    *this << uint8_t(cmSendMail) << message_;
    getCode();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_.sendLastError_ = e->code();
    client_.sendWait_.release();
    throw;
  }
  client_.sendWait_.release();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Client::~Client()
{
}
//------------------------------------------------------------------------------
Client::Client() : pAsyncEvent_(NULL), config_(new ksys::InterlockedConfig<ksys::FiberInterlockedMutex>)
{
  howCloseServer(csTerminate | csShutdown);
  fiberTimeout(0);
}
//------------------------------------------------------------------------------
void Client::open()
{
  AutoPtr<ClientFiber> fiber(new ClientFiber(*this));
  attachFiber(fiber);
  fiber.ptr(NULL);
}
//------------------------------------------------------------------------------
void Client::close()
{
  ksock::Client::close();
}
//------------------------------------------------------------------------------
const utf8::String & Client::newMessage()
{
  return sendQueue_.safeAdd(new Message).id();
}
//------------------------------------------------------------------------------
const utf8::String & Client::value(const utf8::String & id,const utf8::String & key) const
{
  const Vector<Message> * queue = &sendQueue_;
  intptr_t i = queue->bSearch(Message(id));
  if( i < 0 ){
    queue = &recvQueue_;
    i = queue->bSearch(Message(id));
  }
  if( i < 0 )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  return (*queue)[i].value(key);
}
//------------------------------------------------------------------------------
utf8::String Client::value(const utf8::String & id,const utf8::String & key,const utf8::String & value)
{
  Vector<Message> * queue = &sendQueue_;
  intptr_t i = queue->bSearch(Message(id));
  if( i < 0 ){
    queue = &recvQueue_;
    i = queue->bSearch(Message(id));
  }
  if( i < 0 )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  utf8::String oldValue((*queue)[i].value(key));
  (*queue)[i].value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
Client & Client::sendMessage(const utf8::String & id)
{
  intptr_t i = sendQueue_.bSearch(Message(id));
  if( i < 0 )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  sendLastError_ = 0;
  sendQueue_[i].value("#Sender",user_ + "@" + key_);
  sendQueue_[i].value("#Sender.Sended",getTimeString(gettimeofday()));
  sendQueue_[i].value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
  sendQueue_[i].value("#Sender.Process.StartTime",getTimeString(getProcessStartTime()));
  sendQueue_[i].value("#Sender.Host",ksock::SockAddr::gethostname());
  sendWait_.acquire();
  try {
    AutoPtr<ClientMailSenderFiber> fiber(new ClientMailSenderFiber(*this,sendQueue_[i]));
    attachFiber(fiber);
    fiber.ptr(NULL);
  }
  catch( ... ){
    sendWait_.release();
    throw;
  }
  AutoLock<InterlockedMutex> lock(sendWait_);
  return *this;
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
