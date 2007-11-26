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
SerialPortFiber::SerialPortFiber(Client * client,uintptr_t serialPortNumber) :
  client_(client), serialPortNumber_(serialPortNumber), serial_(NULL)
{
}
//------------------------------------------------------------------------------
void SerialPortFiber::removeFromArray()
{
  AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
  intptr_t i = client_->serialPortsFibers_.bSearch(this);
  assert( i >= 0 );
  client_->serialPortsFibers_.remove(i);
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
    serial.readOnly(true).exclusive(true).fileName(
      "COM" + utf8::int2Str(serialPortNumber_) + ":"
    ).open();
#if defined(__WIN32__) || defined(__WIN64__)
    if( SetupComm(serial.descriptor(),1600,1600) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    COMMTIMEOUTS cto;
    if( GetCommTimeouts(serial.descriptor(),&cto) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
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
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( GetCommMask(serial.descriptor(),&evtMask) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( SetCommMask(serial.descriptor(),evtMask | EV_RXCHAR/* | EV_CTS | EV_DSR | EV_ERR | EV_RLSD*/) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
      err = GetLastError() + errorOffset;
      serial.close();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#endif
    AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
    intptr_t c, i = client_->serialPortsFibers_.bSearch(this,c);
    assert( c != 0 );
    client_->serialPortsFibers_.insert(i + (c > 0),this);
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
  }
  try {
    client_->workFiberWait_.release();
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
            client_->sendAsyncEvent(
              client_->name_,
              "BarCodeValue_COM" + utf8::int2Str(serialPortNumber_),
              utf8::String((char *) b.ptr() + j,i - j)
            );
            memcpy(b.ptr(),b.ptr() + i,pos - i);
            //memset(b.ptr() + pos - i,0,b.count() + pos - i);
            pos -= i;
            i = 0;
          }
        }
      }
      removeFromArray();
      client_->workFiberLastError_ = 0;
      client_->workFiberWait_.release();
    }
  }
  catch( ... ){
    removeFromArray();
    if( terminated_ ){
      client_->workFiberLastError_ = 0;
      client_->workFiberWait_.release();
    }
    else {
      throw;
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseClientFiber::~BaseClientFiber()
{
}
//------------------------------------------------------------------------------
BaseClientFiber::BaseClientFiber()
{
}
//------------------------------------------------------------------------------
void BaseClientFiber::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void BaseClientFiber::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
void BaseClientFiber::main()
{
  bool online = false;
  while( !terminated_ ){
    cycleStage0();
    connectHost(online);
    if( cycleStage1() ) break;
    try {
      if( online ){
        onlineStage0();
        onlineStage1();
      }
      else {
        offlineStage0();
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      online = false;
      cycleException(e);
    }
  }
  onTerminate();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientFiber::~ClientFiber()
{
}
//------------------------------------------------------------------------------
ClientFiber::ClientFiber(
  Client * client,
  bool terminateAfterCheck,
  bool waitForMail,
  bool onlyNewMail) :
  client_(client),
  terminateAfterCheck_(terminateAfterCheck),
  waitForMail_(waitForMail),
  onlyNewMail_(onlyNewMail)
{
}
//------------------------------------------------------------------------------
int32_t ClientFiber::getCode2(int32_t noThrowCode0,int32_t noThrowCode1)
{
  int32_t r;
  *this >> r;
  if( r != eOK && r != noThrowCode0 && r != noThrowCode1 )
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r;
}
//------------------------------------------------------------------------------
void ClientFiber::auth()
{
  checkCode(client_->auth(*this));
  recvTimeout(~uint64_t(0));
  sendTimeout(~uint64_t(0));
}
//------------------------------------------------------------------------------
void ClientFiber::cycleStage0()
{
  client_->config_->parse();
  stdErr.rotationThreshold(client_->config_->value("debug_file_rotate_threshold",1024 * 1024));
  stdErr.rotatedFileCount(client_->config_->value("debug_file_rotate_count",10));
  stdErr.setDebugLevels(client_->config_->value("debug_levels","+0,+1,+2,+3"));
}
//------------------------------------------------------------------------------
void ClientFiber::connectHost(bool & online)
{
  if( !online ){
    {
      AutoLock<FiberInterlockedMutex> lock(client_->connectedMutex_);
      client_->connected_ = false;
      client_->connectedToServer_.resize(0);
    }
    close();
    utf8::String server(client_->config_->value("server",client_->mailServer_));
    for( intptr_t i = enumStringParts(server) - 1; i >= 0 && !terminated_ && !online; i-- ){
      try {
        remoteAddress_.resolveName(stringPartByNo(server,i),defaultPort);
        utf8::String fqdn(remoteAddress_.resolveAddr());
        try {
          connect(remoteAddress_);
          try {
            auth();
            try {
              *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
              getCode();
              utf8::String key(client_->config_->value("key",client_->key_));
              try {
                *this << uint8_t(cmRegisterClient) <<
                  UserInfo(client_->user_) << KeyInfo(key);
                utf8::String groups(client_->config_->value("groups",client_->groups_));
                try {
                  uint64_t u = enumStringParts(groups);
                  *this << u;
                  for( intptr_t i = intptr_t(u - 1); i >= 0 && !terminated_; i-- )
                    *this << GroupInfo(stringPartByNo(groups,i));
                  getCode();
                  try {
                    *this << uint8_t(cmRecvMail) << client_->user_ << key << waitForMail_ << onlyNewMail_;
                    getCode();
                    client_->sendAsyncEvent(
                      client_->name_,
                      "Connect",
                      utf8::ptr2Str(this)
                    );
                    {
                      AutoLock<FiberInterlockedMutex> lock(client_->connectedMutex_);
                      client_->connectedToServer_ = fqdn;
                      client_->connected_ = true;
                    }
                    checkMachineBinding(client_->config_->value("machine_key"));
                    online = true;
                  }
                  catch( ExceptionSP & e ){
                    if( terminateAfterCheck_ ){
                      client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
                      terminate();
                    }
                    e->writeStdError();
                    stdErr.debug(3,utf8::String::Stream() <<
                      "Start receiving mail failed.\n"
                    );
                  }
                }
                catch( ExceptionSP & e ){
                  if( terminateAfterCheck_ ){
                    client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
                    terminate();
                  }
                  e->writeStdError();
                  stdErr.debug(3,utf8::String::Stream() <<
                    "Register groups: " << groups << " failed.\n"
                  );
                }
              }
              catch( ExceptionSP & e ){
                if( terminateAfterCheck_ ){
                  client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
                  terminate();
                }
                e->writeStdError();
                stdErr.debug(3,utf8::String::Stream() <<
                  "Register user: " << client_->user_ <<
                  ", key: " << key <<
                  " failed.\n"
                );
              }
            }
            catch( ExceptionSP & e ){
              if( terminateAfterCheck_ ){
                client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
                terminate();
              }
              e->writeStdError();
              stdErr.debug(3,utf8::String::Stream() <<
                "Select server type  " << serverTypeName[stStandalone] <<
                stringPartByNo(server,i) << " failed.\n"
              );
            }
          }
          catch( ExceptionSP & e ){
            if( terminateAfterCheck_ ){
              client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
              terminate();
            }
            e->writeStdError();
            stdErr.debug(3,utf8::String::Stream() <<
              "Authentification to host " <<
              stringPartByNo(server,i) << " failed.\n"
            );
          }
        }
        catch( ExceptionSP & e ){
          if( terminateAfterCheck_ ){
            client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
            terminate();
          }
          e->writeStdError();
          stdErr.debug(3,utf8::String::Stream() <<
            "Unable to connect. Host " <<
            stringPartByNo(server,i) <<
            " unreachable.\n"
          );
        }
      }
      catch( ExceptionSP & e ){
        if( terminateAfterCheck_ ){
          client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
          terminate();
        }
        e->writeStdError();
        stdErr.debug(3,utf8::String::Stream() <<
          "Unable to resolve host: " << stringPartByNo(server,i) << "\n"
        );
      }
    }
  }
}
//------------------------------------------------------------------------------
void ClientFiber::onlineStage0()
{
  message_ = newObject<Message>();
  message_->file().createIfNotExist(true).removeAfterClose(true);
  if( !waitForMail_ ){
    bool newMessage;
    *this >> newMessage;
    if( !newMessage ){
      terminate();
      return;
    }
  }
  *this >> message_;
}
//------------------------------------------------------------------------------
void ClientFiber::onlineStage1()
{
  if( terminated_ ) return;
  Message * msg;
  utf8::String msgId(message_->id());
  {
    AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
    msg = client_->recvQueue_.find(message_);
    if( msg == NULL ){
      client_->recvQueue_.insert(*message_.ptr());
      message_.ptr(NULL);
    }
  }
  bool messageAccepted = true;
  if( msg == NULL ){
    HRESULT hr = client_->sendAsyncEvent(
      client_->name_,
      "Message",
      msgId
    );
    if( FAILED(hr) ){
      messageAccepted = false;
      AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
      Message * msg = client_->recvQueue_.find(msgId);
      if( msg != NULL ) client_->recvQueue_.drop(*msg);
    }
  }
  *this << messageAccepted;
  getCode2(eLastMessage);
}
//------------------------------------------------------------------------------
void ClientFiber::cycleException(ExceptionSP & e)
{
  if( terminateAfterCheck_ ){
    client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
  }
  else {
    client_->sendAsyncEvent(
      client_->name_,
      "Disconnect",
      utf8::ptr2Str(this)
    );
  }
}
//------------------------------------------------------------------------------
void ClientFiber::onTerminate()
{
  if( terminateAfterCheck_ ) client_->workFiberWait_.release();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientMailFiber::~ClientMailFiber()
{
}
//------------------------------------------------------------------------------
ClientMailFiber::ClientMailFiber(Client * client) :
  client_(client), messagesAutoDrop_(messages_)
{
}
//------------------------------------------------------------------------------
void ClientMailFiber::removeMessage(MessageControl * message)
{
  AutoLock<FiberInterlockedMutex> lock(messageMutex_);
  EmbeddedListNode<MessageControl> * p = messages_.first();
  while( p != NULL ){
    if( message == &MessageControl::nodeObject(*p) ){
      messages_.drop(*p);
      break;
    }
    p = p->next();
  }
}
//------------------------------------------------------------------------------
void ClientMailFiber::auth()
{
  checkCode(client_->auth(*this));
}
//------------------------------------------------------------------------------
void ClientMailFiber::connectHost(bool & online)
{
  if( !online ){
    close();
    utf8::String server(client_->config_->value("server",client_->mailServer_));
    for( intptr_t i = enumStringParts(server) - 1; i >= 0 && !terminated_; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveName(stringPartByNo(server,i),defaultPort);
        utf8::String fqdn(remoteAddress.resolveAddr());
        try {
          connect(remoteAddress);
          try {
            auth();
            *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
            getCode();
            online = true;
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
            stdErr.debug(3,utf8::String::Stream() <<
              "Authentification to host " << fqdn << " failed.\n"
            );
          }
        }
        catch( ExceptionSP & e ){
          e->writeStdError();
          stdErr.debug(2,utf8::String::Stream() <<
            "Unable to connect. Host " << fqdn << " unreachable.\n"
          );
        }
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        stdErr.debug(3,utf8::String::Stream() <<
          "Unable to resolve host: " << stringPartByNo(server,i) << "\n"
        );
      }
    }
  }
}
//------------------------------------------------------------------------------
void ClientMailFiber::newMessage()
{
  message_ = NULL;
  AutoLock<FiberInterlockedMutex> lock(messageMutex_);
  EmbeddedListNode<MessageControl> * p = messages_.first();
  while( p != NULL ){
    if( !MessageControl::nodeObject(*p).async_ ){
      message_ = &MessageControl::nodeObject(*p);
      break;
    }
    p = p->next();
  }
  if( message_ == NULL ) message_ = &MessageControl::nodeObject(*messages_.first());
}
//------------------------------------------------------------------------------
void ClientMailFiber::onTerminate()
{
  AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
  client_->clientMailFiber_ = NULL;
}
//------------------------------------------------------------------------------
bool ClientMailFiber::cycleStage1()
{
  bool r = false;
  newMessage();
  if( message_ == NULL ){
    uint64_t inactivityTime = client_->config_->value("fiber_inactivity_time",60u);
    if( !semaphore_.timedWait(inactivityTime * 1000000u) ){
      AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
      client_->clientMailFiber_ = NULL;
      newMessage();
      if( message_ == NULL ){
        r = true;
      }
      else {
        client_->clientMailFiber_ = this;
      }
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void ClientMailFiber::cycleException(ExceptionSP & e)
{
  if( message_ != NULL ){
    utf8::String ev;
    switch( message_->operation_ ){
      case MessageControl::msgNone : assert( 0 ); break;
      case MessageControl::msgSend :
        ev = "MessageSendingError_";
        break;
      case MessageControl::msgRemove :
        ev = "MessageRemovingError_";
        break;
    }
    if( message_->async_ ){
      client_->sendAsyncEvent(
        client_->name_,
        ev + utf8::int2Str(e->code() - (e->code() >= errorOffset ? errorOffset : 0)),
        message_->message_->id()
      );
      AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
      switch( message_->operation_ ){
        case MessageControl::msgNone : assert( 0 ); break;
        case MessageControl::msgSend :
          client_->sendQueue_.drop(*message_->message_);
          break;
        case MessageControl::msgRemove :
          client_->recvQueue_.drop(*message_->message_);
          break;
      }
    }
    else {
      client_->workFiberLastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
      client_->workFiberWait_.release();
    }
    removeMessage(message_);
    message_ = NULL;
  }
}
//------------------------------------------------------------------------------
void ClientMailFiber::onlineStage0()
{
  if( message_ != NULL ){
    switch( message_->operation_ ){
      case MessageControl::msgNone : assert( 0 ); break;
      case MessageControl::msgSend :
        message_->message_->value("#Sender",client_->user_ + "@" + client_->config_->value("key",client_->key_));
        message_->message_->value("#Sender.Sended",getTimeString(gettimeofday()));
        message_->message_->value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
        message_->message_->value("#Sender.Process.StartTime",getTimeString(getProcessStartTime()));
        message_->message_->value("#Sender.Host",ksock::SockAddr::gethostname());
        *this << uint8_t(cmSendMail) << message_->message_->id() << false /* no rest flag */ << *message_->message_;
        getCode();
        if( message_->async_ ){
          client_->sendAsyncEvent(
            client_->name_,
            "MessageSended",
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
          client_->sendQueue_.drop(*message_->message_);
        }
        else {
          client_->workFiberWait_.release();
        }
        removeMessage(message_);
        message_ = NULL;
        break;
      case MessageControl::msgRemove :
        *this << uint8_t(cmRemoveMail) << client_->user_ << message_->message_->id();
        getCode();
        if( message_->async_ ){
          client_->sendAsyncEvent(
            client_->name_,
            "MessageRemoved",
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
          client_->recvQueue_.drop(*message_->message_);
        }
        else {
          client_->workFiberWait_.release();
        }
        removeMessage(message_);
        message_ = NULL;
        break;
    }
  }
}
//------------------------------------------------------------------------------
void ClientMailFiber::offlineStage0()
{
  if( message_ != NULL ){
    switch( message_->operation_ ){
      case MessageControl::msgNone : assert( 0 ); break;
      case MessageControl::msgSend :
        if( message_->async_ ){
          client_->sendAsyncEvent(
            client_->name_,
            "MessageSendingError_" + utf8::int2Str(WSAECONNREFUSED),
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
          client_->sendQueue_.drop(*message_->message_);
        }
        else {
          client_->workFiberLastError_ = WSAECONNREFUSED;
          client_->workFiberWait_.release();
        }
        removeMessage(message_);
        message_ = NULL;
        break;
      case MessageControl::msgRemove :
        if( message_->async_ ){
          client_->sendAsyncEvent(
            client_->name_,
            "MessageRemovingError_" + utf8::int2Str(WSAECONNREFUSED),
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_->queueMutex_);
          client_->recvQueue_.drop(*message_->message_);
        }
        else {
          client_->workFiberLastError_ = WSAECONNREFUSED;
          client_->workFiberWait_.release();
        }
        removeMessage(message_);
        message_ = NULL;
        break;
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientDBGetterFiber::~ClientDBGetterFiber()
{
}
//------------------------------------------------------------------------------
ClientDBGetterFiber::ClientDBGetterFiber(Client & client) : ClientFiber(&client)
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
      AutoLock<FiberInterlockedMutex> lock(client_->connectedMutex_);
      registered = client_->connected_ || !client_->asyncMessagesReceiving_;
    }
    if( !registered )
      newObjectV1C2<Exception>(ERROR_CONNECTION_UNAVAIL + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    utf8::String server(client_->config_->value("server",client_->mailServer_));
    for( i = enumStringParts(server) - 1; i >= 0; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolveName(stringPartByNo(server,i),defaultPort);
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
    client_->data_.clear().ore(tdata);
    bool isEmpty = 
      client_->data_.getUserList().strlen() == 0 &&
      client_->data_.getKeyList().strlen() == 0
    ;
    client_->sendAsyncEvent(client_->name_,"GetDB",utf8::String(isEmpty ? "" : "DATA"));
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_->sendAsyncEvent(
      client_->name_,
      "GetDB",
      utf8::int2Str(e->code() - (e->code() >= errorOffset) * errorOffset)
    );
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MK1100ClientFiber::~MK1100ClientFiber()
{
}
//------------------------------------------------------------------------------
MK1100ClientFiber::MK1100ClientFiber(Client * client,MK1100TCPServer * server) :
  client_(client), server_(server)
{
}
//------------------------------------------------------------------------------
utf8::String MK1100ClientFiber::readString()
{
	int l;
  readBuffer(&l,sizeof(l));
  AutoPtr<wchar_t> s;
  s.alloc((l + 1) * sizeof(wchar_t));
  readBuffer(s,l * sizeof(wchar_t));
  s[l] = L'\0';
	return s.ptr();
}
//------------------------------------------------------------------------------
MK1100ClientFiber & MK1100ClientFiber::writeString(const utf8::String & s)
{
  utf8::WideString ws(s.getUNICODEString());
  int l = (int) wcslen(ws);
  writeBuffer(&l,sizeof(l)).writeBuffer(ws,l * sizeof(wchar_t));
	return *this;
}
//------------------------------------------------------------------------------
void MK1100ClientFiber::fiberBreakExecution()
{
  sem_.post().post();
}
//------------------------------------------------------------------------------
void MK1100ClientFiber::mainHelper()
{
  AutoLock<FiberInterlockedMutex> lock(server_->fibersMutex_);
  intptr_t i = server_->fibers_.bSearch(this);
  if( i >= 0 ) server_->fibers_.remove(i);
}
//------------------------------------------------------------------------------
void MK1100ClientFiber::main()
{
  {
    AutoLock<FiberInterlockedMutex> lock(server_->fibersMutex_);
    intptr_t c, i = server_->fibers_.bSearch(this,c);
    server_->fibers_.insert(i + (c > 0),this);
  }
  try {
    utf8::String barCode, barCodeType;
    while( !terminated_ ){
      barCode = readString();
      barCodeType = readString();
      client_->sendAsyncEvent(
        client_->name_,
        "MK1100GETBARCODEINFO_" + utf8::ptr2Str(this),
        barCodeType + "_" + barCode
      );
      sem_.wait();
      int l = int(enumStringParts(data_));
      writeBuffer(&l,sizeof(l));
      for( int i = 0; i < l; i += 2 ){
        writeString(stringPartByNo(data_,i));
        writeString(stringPartByNo(data_,i + 1));
      }
    }
  }
  catch( ExceptionSP & ){
    mainHelper();
    throw;
  }
  mainHelper();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MK1100TCPServer::~MK1100TCPServer()
{
}
//------------------------------------------------------------------------------
MK1100TCPServer::MK1100TCPServer(Client * client) :
  client_(client)
{
  howCloseServer(howCloseServer() & ~csWait);
}
//------------------------------------------------------------------------------
Fiber * MK1100TCPServer::newFiber()
{
  return newObjectV1V2<MK1100ClientFiber>(client_,this);
}
//------------------------------------------------------------------------------
void MK1100TCPServer::open()
{
  Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolveNameForBind("",addrs,client_->mk1100Port_);
  for( uintptr_t i = 0; i < addrs.count(); i++ ) addBind(addrs[i]);
  ksock::Server::open();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Client::~Client()
{
  close();
}
//------------------------------------------------------------------------------
Client::Client(const Client &) :
  config_(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()),
  recvQueueAutoDrop_(recvQueue_),
  sendQueueAutoDrop_(sendQueue_)
{
  assert( 0 );
  newObjectV1C2<Exception>(ERROR_CALL_NOT_IMPLEMENTED + errorOffset,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
Client::Client() :
  pAsyncEvent_(NULL),
  config_(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()),
  recvQueueAutoDrop_(recvQueue_),
  sendQueueAutoDrop_(sendQueue_),
  connected_(false),
  asyncMessagesReceiving_(true),
  mk1100Port_(0),
  clientMailFiber_(NULL)
{
  howCloseServer(csTerminate | csShutdown | csAbort);
  fiberTimeout(0);
}
//------------------------------------------------------------------------------
void Client::open()
{
  data_.clear();
  ftime_ = 0;
  if( asyncMessagesReceiving_ )
    attachFiber(newObjectV1<ClientFiber>(this));
}
//------------------------------------------------------------------------------
void Client::close()
{
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    if( clientMailFiber_ != NULL ){
      clientMailFiber_->terminate();
      clientMailFiber_->semaphore_.post();
    }
  }
  removeAllSerialPortScanners();
  ksock::Client::close();
  sendQueue_.drop();
  recvQueue_.drop();
}
//------------------------------------------------------------------------------
bool Client::receiveMessages(bool onlyNewMail)
{
  workFiberLastError_ = 0;
  workFiberWait_.acquire();
  attachFiber(newObjectV1V2V3V4<ClientFiber>(this,true,false,onlyNewMail));
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
utf8::String Client::newMessage()
{
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  Message * msg = newObject<Message>();
  sendQueue_.insert(*msg);
  msg->file().fileName(
    includeTrailingPathDelimiter(getPathFromPathName(msg->file().fileName())) + 
    changeFileExt(getNameFromPathName(msg->file().fileName()),utf8::String()) + "#.msg"
  );
  msg->file().createIfNotExist(true).removeAfterClose(true);
  return msg->id();
}
//------------------------------------------------------------------------------
HRESULT Client::value(const utf8::String id,const utf8::String key,VARIANT * pvarRetValue) const
{
  Message::validateKey(key);
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  const Messages * queue = &sendQueue_;
  const Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL )
    newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  HRESULT hr = S_OK;
  utf8::String value;
  if( msg->isValue(key,&value) ){
    V_BSTR(pvarRetValue) = value.getOLEString();
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
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  Message * msg = sendQueue_.find(id);
  if( msg == NULL ) newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  utf8::String oldValue;
  msg->isValue(key,&oldValue);
  msg->value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
bool Client::attachFileToMessage(const utf8::String id,const utf8::String key,const utf8::String fileName)
{
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    Message * msg = sendQueue_.find(id);
    if( msg == NULL ) newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    if( msg->isValue(key) )
      newObjectV1C2<Exception>(ERROR_ALREADY_EXISTS + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  AsyncFile file;
  file.fileName(fileName).readOnly(true).open();
  size_t bl = getpagesize() * 16;
  AutoPtr<uint8_t> b;
  b.alloc(bl);
  uint64_t i = 0, l = 0, ll;
  for( ll = file.size(); ll > 0; ll -= l, i++ ){
    l = ll > bl ? bl : ll;
    file.readBuffer(b,l);
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    msg = sendQueue_.find(id);
    if( msg == NULL ) newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    msg->value(key + "#" + utf8::int2HexStr(i,8),base64Encode(b,uintptr_t(l)));
  }
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  msg = sendQueue_.find(id);
  if( msg == NULL ) newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  msg->value(key,"attached file '" + fileName + "', original size " + utf8::int2Str(file.size()) + ", block count " + utf8::int2Str(i));
  return true;
}
//------------------------------------------------------------------------------
bool Client::saveMessageAttachmentToFile(const utf8::String id,const utf8::String key,const utf8::String fileName)
{
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    Message * msg = recvQueue_.find(id);
    if( msg == NULL || !msg->isValue(key) )
      newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  AsyncFile file;
  file.fileName(fileName).createIfNotExist(true).open();
  AutoPtr<uint8_t> b;
  for( uint64_t i = 0; i < ~uint64_t(0); i++ ){
    utf8::String bkey(key + "#" + utf8::int2HexStr(i,8)), v;
    {
      AutoLock<FiberInterlockedMutex> lock(queueMutex_);
      msg = recvQueue_.find(id);
      if( msg == NULL || !msg->isValue(key) )
        newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      if( !msg->isValue(bkey,&v) ) break;
    }
    uintptr_t l = base64Decode(v,NULL,0);
    b.realloc(l);
    base64Decode(v,b,l);
    file.writeBuffer(b,l);
  }
  file.resize(file.tell());
  return true;
}
//------------------------------------------------------------------------------
bool Client::sendMessage(const utf8::String id,bool async)
{
//  checkMachineBinding(config_->value("machine_key"));
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    msg = sendQueue_.find(id);
    if( msg == NULL ) return false;
  }
  if( !async ) workFiberWait_.acquire();
  workFiberLastError_ = 0;
  try {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    if( clientMailFiber_ == NULL ){
      ClientMailFiber * clientMailFiber;
      attachFiber(clientMailFiber = newObjectV1<ClientMailFiber>(this));
      clientMailFiber_ = clientMailFiber;
    }
    AutoLock<FiberInterlockedMutex> lock2(clientMailFiber_->messageMutex_);
    clientMailFiber_->messages_.insToTail(
      *newObjectV1V2V3<ClientMailFiber::MessageControl>(msg,ClientMailFiber::MessageControl::msgSend,async)
    );
    if( clientMailFiber_->messages_.count() == 1 ) clientMailFiber_->semaphore_.post();
  }
  catch( ... ){
    if( !async ) workFiberWait_.release();
    throw;
  }
  if( async ) return true;
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  if( workFiberLastError_ == 0 ){
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    sendQueue_.drop(*msg);
  }
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
bool Client::removeMessage(const utf8::String id,bool async)
{
  Messages * queue = &sendQueue_;
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    msg = sendQueue_.find(id);
    if( msg != NULL ){
      sendQueue_.drop(*msg);
      return true;
    }
    msg = recvQueue_.find(id);
    if( msg == NULL ) return false;
  }
  if( !async ) workFiberWait_.acquire();
  workFiberLastError_ = 0;
  try {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    if( clientMailFiber_ == NULL ){
      ClientMailFiber * clientMailFiber;
      attachFiber(clientMailFiber = newObjectV1<ClientMailFiber>(this));
      clientMailFiber_ = clientMailFiber;
    }
    AutoLock<FiberInterlockedMutex> lock2(clientMailFiber_->messageMutex_);
    clientMailFiber_->messages_.insToTail(
      *newObjectV1V2V3<ClientMailFiber::MessageControl>(msg,ClientMailFiber::MessageControl::msgRemove,async)
    );
    if( clientMailFiber_->messages_.count() == 1 ) clientMailFiber_->semaphore_.post();
  }
  catch( ... ){
    if( !async) workFiberWait_.release();
    throw;
  }
  if( async ) return true;
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  if( workFiberLastError_ == 0 ){
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    msg = recvQueue_.find(id);
    assert( msg != NULL );
    recvQueue_.drop(*msg);
  }
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
utf8::String Client::getReceivedMessageList() const
{
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  utf8::String slist;
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
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
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
  attachFiber(newObjectR1<ClientDBGetterFiber>(*this));
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
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    const Messages * queue = &sendQueue_;
    msg = queue->find(id);
    if( msg == NULL ){
      queue = &recvQueue_;
      msg = queue->find(id);
    }
  }
  if( msg == NULL )
    newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  AutoPtr<Message> message(newObject<Message>());
  utf8::String newId(message->id());
  message->file().fileName(
    includeTrailingPathDelimiter(getPathFromPathName(message->file().fileName())) + 
    newId + "#.msg"
  );
  message->file().createIfNotExist(true).removeAfterClose(true).open() << *msg;
  message->file().seek(0) >> message;
  message->removeValueByLeft("#Relay");
  message->id(newId);
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  sendQueue_.insert(*message.ptr());
  message.ptr(NULL);
  return newId;
}
//------------------------------------------------------------------------------
utf8::String Client::removeValue(const utf8::String id,const utf8::String key)
{
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  Messages * queue = &sendQueue_;
  Message * msg = queue->find(id);
  if( msg == NULL ){
    queue = &recvQueue_;
    msg = queue->find(id);
  }
  if( msg == NULL )
    newObjectV1C2<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return msg->removeValue(key);
}
//------------------------------------------------------------------------------
bool Client::installSerialPortScanner(uintptr_t serialPortNumber)
{
  bool installed = false;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
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
      attachFiber(newObjectV1V2<SerialPortFiber>(this,serialPortNumber));
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
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    for( i = serialPortsFibers_.count() - 1; i >= 0; i-- )
      if( serialPortsFibers_[i]->serialPortNumber_ == serialPortNumber ){
        workFiberLastError_ = 0;
        workFiberWait_.acquire();
        serialPortsFibers_[i]->terminate();
        serialPortsFibers_[i]->serial_->close();
        break;
      }
  }
  if( i < 0 ) return false;
  AutoLock<InterlockedMutex> lock(workFiberWait_);
  return workFiberLastError_ == 0;
}
//------------------------------------------------------------------------------
void Client::removeAllSerialPortScanners()
{
  for(;;){
    intptr_t i;
    {
      AutoLock<FiberInterlockedMutex> lock(queueMutex_);
      i = serialPortsFibers_.count() - 1;
      if( i >= 0 ){
        workFiberLastError_ = 0;
        workFiberWait_.acquire();
        serialPortsFibers_[i]->terminate();
        serialPortsFibers_[i]->serial_->close();
      }
    }
    if( i < 0 ) break;
    AutoLock<InterlockedMutex> lock(workFiberWait_);
  }
}
//------------------------------------------------------------------------------
HRESULT Client::sendAsyncEvent(const utf8::String & source,const utf8::String & event,const utf8::String & data)
{
  HRESULT hr = S_OK;
  BSTR source0 = NULL, event0 = NULL, data0 = NULL;
  try {
    source0 = source.getOLEString();
    event0 = event.getOLEString();
    data0 = data.getOLEString();
  }
  catch( ... ){
    SysFreeString(data0);
    SysFreeString(event0);
    SysFreeString(source0);
    hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
  }
  if( SUCCEEDED(hr) ){
    hr = pAsyncEvent_->ExternalEvent(source0,event0,data0);
    if( FAILED(hr) ){
      ExceptionSP e(
        newObjectV1C2<Exception>(
          HRESULT_CODE(hr) + errorOffset,
          utf8::String(__PRETTY_FUNCTION__) + " line " +
          utf8::int2Str(__LINE__) + ", pAsyncEvent_->ExternalEvent(" +
          source + "," + event + "," + data + ") failed"
        )
      );
      e->writeStdError();
    }
  }
  return hr;
}
//------------------------------------------------------------------------------
int32_t Client::auth(ksock::AsyncSocket & socket)
{
  ksock::AsyncSocket::AuthParams ap;
  ap.maxSendSize_ = config_->value("max_send_size",-1);
  ap.maxRecvSize_ = config_->value("max_recv_size",-1);
  ap.recvTimeout_ = config_->value("recv_timeout",-1);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = config_->value("send_timeout",-1);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.user_ = config_->text("user","system");
  ap.password_ = config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  ap.encryption_ = config_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = config_->section("encryption").value("threshold",1024 * 1024);
  ap.compression_ = config_->section("compression").text(utf8::String(),"default");
  ap.compressionType_ = config_->section("compression").value("type","default");
  ap.crc_ = config_->section("compression").value("crc","default");
  ap.level_ = config_->section("compression").value("level",9);
  ap.optimize_ = config_->section("compression").value("optimize",true);
  ap.bufferSize_ = config_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.noAuth_ = config_->value("noauth",false);
  return socket.clientAuth(ap);
}
//------------------------------------------------------------------------------
Client & Client::readConfig(const utf8::String & configFile,const utf8::String & logFile)
{
  configFile_ = configFile;
  logFile_ = logFile;
  stdErr.fileName(logFile_);
  config_->fileName(configFile_).silent(true).parse();
  stdErr.bufferDataTTA(
    (uint64_t) config_->value("debug_file_max_collection_time",60) * 1000000u
  );
  stdErr.rotationThreshold(config_->value("debug_file_rotate_threshold",1024 * 1024));
  stdErr.rotatedFileCount(config_->value("debug_file_rotate_count",10));
  stdErr.setDebugLevels(config_->value("debug_levels","+0,+1,+2,+3"));
  stdErr.fileName(
    config_->value("log_file",stdErr.fileName())
  );
  stackBackTrace = config_->value("stack_back_trace",true);
  config_->silent(false);
  return *this;
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
