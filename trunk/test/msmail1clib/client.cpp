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
  AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
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
    serial.readOnly(true).exclusive(true).fileName(
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
    AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
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
            client_.sendAsyncEvent(
              client_.name_,
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
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
ClientFiber::ClientFiber(Client & client) : client_(client)
{
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
  checkCode(client_.auth(*this));
}
//------------------------------------------------------------------------------
void ClientFiber::cycleStage0()
{
  client_.config_->parse();
  stdErr.rotationThreshold(client_.config_->value("debug_file_rotate_threshold",1024 * 1024));
  stdErr.rotatedFileCount(client_.config_->value("debug_file_rotate_count",10));
  stdErr.setDebugLevels(client_.config_->value("debug_levels","+0,+1,+2,+3"));
}
//------------------------------------------------------------------------------
void ClientFiber::connectHost(bool & online)
{
  if( !online ){
    {
      AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
      client_.connected_ = false;
      client_.connectedToServer_.resize(0);
    }
    close();
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( intptr_t i = enumStringParts(server) - 1; i >= 0 && !terminated_ && !online; i-- ){
      try {
        remoteAddress_.resolve(stringPartByNo(server,i),defaultPort);
        utf8::String fqdn(remoteAddress_.resolve());
        try {
          connect(remoteAddress_);
          try {
            auth();
            try {
              *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
              getCode();
              utf8::String key(client_.config_->value("key",client_.key_));
              try {
                *this << uint8_t(cmRegisterClient) <<
                  UserInfo(client_.user_) << KeyInfo(key);
                utf8::String groups(client_.config_->value("groups",client_.groups_));
                try {
                  uint64_t u = enumStringParts(groups);
                  *this << u;
                  for( intptr_t i = intptr_t(u - 1); i >= 0 && !terminated_; i-- )
                    *this << GroupInfo(stringPartByNo(groups,i));
                  getCode();
                  try {
                    *this << uint8_t(cmRecvMail) << client_.user_ << key << bool(true) << bool(true);
                    getCode();
                    client_.sendAsyncEvent(
                      client_.name_,
                      "Connect",
                      utf8::ptr2Str(this)
                    );
                    {
                      AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
                      client_.connectedToServer_ = fqdn;
                      client_.connected_ = true;
                    }
                    checkMachineBinding(client_.config_->value("machine_key"));
                    online = true;
                  }
                  catch( ExceptionSP & e ){
                    e->writeStdError();
                    stdErr.debug(3,utf8::String::Stream() <<
                      "Start receiving mail failed.\n"
                    );
                  }
                }
                catch( ExceptionSP & e ){
                  e->writeStdError();
                  stdErr.debug(3,utf8::String::Stream() <<
                    "Register groups: " << groups << " failed.\n"
                  );
                }
              }
              catch( ExceptionSP & e ){
                e->writeStdError();
                stdErr.debug(3,utf8::String::Stream() <<
                  "Register user: " << client_.user_ <<
                  ", key: " << key <<
                  " failed.\n"
                );
              }
            }
            catch( ExceptionSP & e ){
              e->writeStdError();
              stdErr.debug(3,utf8::String::Stream() <<
                "Select server type  " << serverTypeName_[stStandalone] <<
                stringPartByNo(server,i) << " failed.\n"
              );
            }
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
            stdErr.debug(3,utf8::String::Stream() <<
              "Authentification to host " <<
              stringPartByNo(server,i) << " failed.\n"
            );
          }
        }
        catch( ExceptionSP & e ){
          e->writeStdError();
          stdErr.debug(3,utf8::String::Stream() <<
            "Unable to connect. Host " <<
            stringPartByNo(server,i) <<
            " unreachable.\n"
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
void ClientFiber::onlineStage0()
{
  message_ = newObject<Message>();
  message_->file().createIfNotExist(true).removeAfterClose(true);
  *this >> message_;
}
//------------------------------------------------------------------------------
void ClientFiber::onlineStage1()
{
  Message * msg;
  utf8::String msgId(message_->id());
  {
    AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
    msg = client_.recvQueue_.find(message_);
    if( msg == NULL ){
      client_.recvQueue_.insert(*message_.ptr());
      message_.ptr(NULL);
    }
  }
  bool messageAccepted = true;
  if( msg == NULL ){
    HRESULT hr = client_.sendAsyncEvent(
      client_.name_,
      "Message",
      msgId
    );
    if( FAILED(hr) ){
      messageAccepted = false;
      AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
      Message * msg = client_.recvQueue_.find(msgId);
      if( msg != NULL ) client_.recvQueue_.drop(*msg);
    }
  }
  *this << messageAccepted;
  getCode2(eLastMessage);
}
//------------------------------------------------------------------------------
void ClientFiber::cycleException(ExceptionSP &)
{
  client_.sendAsyncEvent(
    client_.name_,
    "Disconnect",
    utf8::ptr2Str(this)
  );
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ClientMailFiber::~ClientMailFiber()
{
}
//------------------------------------------------------------------------------
ClientMailFiber::ClientMailFiber(Client & client) : client_(client)
{
}
//------------------------------------------------------------------------------
void ClientMailFiber::removeMessage(MessageControl * message)
{
  AutoLock<FiberInterlockedMutex> lock(messageMutex_);
  for( uintptr_t i = 0; i < messages_.count(); i++ )
    if( message == &messages_[i] ) messages_.remove(i);
}
//------------------------------------------------------------------------------
void ClientMailFiber::auth()
{
  checkCode(client_.auth(*this));
}
//------------------------------------------------------------------------------
void ClientMailFiber::connectHost(bool & online)
{
  if( !online ){
    close();
    utf8::String server(client_.config_->value("server",client_.mailServer_));
    for( intptr_t i = enumStringParts(server) - 1; i >= 0 && !terminated_; i-- ){
      ksock::SockAddr remoteAddress;
      try {
        remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
        utf8::String fqdn(remoteAddress.resolve());
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
  for( uintptr_t i = 0; i < messages_.count(); i++ ){
    if( !messages_[i].async_ ){
      message_ = &messages_[i];
      break;
    }
  }
  if( message_ == NULL && messages_.count() > 0 ) message_ = &messages_[0];
}
//------------------------------------------------------------------------------
void ClientMailFiber::onTerminate()
{
  AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
  client_.clientMailFiber_ = NULL;
}
//------------------------------------------------------------------------------
bool ClientMailFiber::cycleStage1()
{
  bool r = false;
  newMessage();
  if( message_ == NULL ){
    uint64_t inactivityTime = client_.config_->value("fiber_inactivity_time",60);
    if( !semaphore_.timedWait(inactivityTime * 1000000u) ){
      AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
      client_.clientMailFiber_ = NULL;
      newMessage();
      if( message_ == NULL ){
        r = true;
      }
      else {
        client_.clientMailFiber_ = this;
      }
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void ClientMailFiber::onlineStage0()
{
  if( message_ != NULL ){
    switch( message_->operation_ ){
      case MessageControl::msgNone : assert( 0 ); break;
      case MessageControl::msgSend :
        *this << uint8_t(cmSendMail) << message_->message_->id() << false /* no rest flag */ << *message_->message_;
        getCode();
        if( message_->async_ ){
          client_.sendAsyncEvent(
            client_.name_,
            "MessageSended",
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
          client_.recvQueue_.drop(*message_->message_);
        }
        else {
          client_.workFiberWait_.release();
        }
        removeMessage(message_);
        break;
      case MessageControl::msgRemove :
        *this << uint8_t(cmRemoveMail) << client_.user_ << message_->message_->id();
        getCode();
        if( message_->async_ ){
          client_.sendAsyncEvent(
            client_.name_,
            "MessageRemoved",
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
          client_.recvQueue_.drop(*message_->message_);
        }
        else {
          client_.workFiberWait_.release();
        }
        removeMessage(message_);
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
          client_.sendAsyncEvent(
            client_.name_,
            "MessageSendingError_" + utf8::int2Str(WSAECONNREFUSED),
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
          client_.sendQueue_.drop(*message_->message_);
        }
        else {
          client_.workFiberLastError_ = WSAECONNREFUSED + errorOffset;
          client_.workFiberWait_.release();
        }
        removeMessage(message_);
        break;
      case MessageControl::msgRemove :
        if( message_->async_ ){
          client_.sendAsyncEvent(
            client_.name_,
            "MessageRemovingError_" + utf8::int2Str(WSAECONNREFUSED),
            message_->message_->id()
          );
          AutoLock<FiberInterlockedMutex> lock(client_.queueMutex_);
          client_.recvQueue_.drop(*message_->message_);
        }
        else {
          client_.workFiberLastError_ = WSAECONNREFUSED + errorOffset;
          client_.workFiberWait_.release();
        }
        removeMessage(message_);
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
    bool isEmpty = 
      client_.data_.getUserList().strlen() == 0 &&
      client_.data_.getKeyList().strlen() == 0
    ;
    client_.sendAsyncEvent(client_.name_,"GetDB",utf8::String(isEmpty ? "" : "DATA"));
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    client_.sendAsyncEvent(
      client_.name_,
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
  close();
}
//------------------------------------------------------------------------------
Client::Client() :
  pAsyncEvent_(NULL),
  config_(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()),
  recvQueueAutoDrop_(recvQueue_),
  sendQueueAutoDrop_(sendQueue_),
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
  attachFiber(newObjectV<ClientFiber>(*this));
  if( mk1100Port_ != 0 )
    attachFiber(newObjectV<MK1100ClientFiber>(*this));
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
utf8::String Client::newMessage()
{
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  Message * msg = newObject<Message>();
  sendQueue_.insert(*msg);
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
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
  Message * msg = sendQueue_.find(id);
  if( msg == NULL ) newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  utf8::String oldValue;
  if( msg->isValue(key) ) oldValue = msg->value(key);
  msg->value(key,value);
  return oldValue;
}
//------------------------------------------------------------------------------
bool Client::sendMessage(const utf8::String id,bool async)
{
  checkMachineBinding(config_->value("machine_key"));
  Message * msg;
  {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    msg = sendQueue_.find(id);
    if( msg == NULL ) return false;
    msg->value("#Sender",user_ + "@" + config_->value("key",key_));
    msg->value("#Sender.Sended",getTimeString(gettimeofday()));
    msg->value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
    msg->value("#Sender.Process.StartTime",getTimeString(getProcessStartTime()));
    msg->value("#Sender.Host",ksock::SockAddr::gethostname());
  }
  if( !async ) workFiberWait_.acquire();
  workFiberLastError_ = 0;
  try {
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
    if( clientMailFiber_ == NULL ){
      ClientMailFiber * clientMailFiber;
      attachFiber(clientMailFiber = newObjectV<ClientMailFiber>(*this));
      clientMailFiber_ = clientMailFiber;
    }
    AutoLock<FiberInterlockedMutex> lock2(clientMailFiber_->messageMutex_);
    clientMailFiber_->messages_.add(ClientMailFiber::MessageControl(msg,ClientMailFiber::MessageControl::msgSend,async));
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
      attachFiber(clientMailFiber = newObjectV<ClientMailFiber>(*this));
      clientMailFiber_ = clientMailFiber;
    }
    AutoLock<FiberInterlockedMutex> lock2(clientMailFiber_->messageMutex_);
    clientMailFiber_->messages_.add(ClientMailFiber::MessageControl(msg,ClientMailFiber::MessageControl::msgRemove,async));
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
  AutoLock<FiberInterlockedMutex> lock(queueMutex_);
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
  message->file().createIfNotExist(true).removeAfterClose(true).open() << *msg >> message;
  message->removeValueByLeft("#Relay");
  message->id(newId);
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
    newObject<Exception>(ERROR_NOT_FOUND + errorOffset,__PRETTY_FUNCTION__)->throwSP();
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
    AutoLock<FiberInterlockedMutex> lock(queueMutex_);
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
      AutoLock<FiberInterlockedMutex> lock(queueMutex_);
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
        newObject<Exception>(
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
  utf8::String user, password, encryption, compression, compressionType, crc;
  socket.maxRecvSize(config_->value("max_recv_size",-1));
  socket.maxSendSize(config_->value("max_send_size",-1));
  user = config_->text("user","system");
  password = config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  encryption = config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = config_->section("encryption").value("threshold",1024 * 1024);
  compression = config_->section("compression").text(utf8::String(),"default");
  compressionType = config_->section("compression").value("type","default");
  crc = config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = config_->section("compression").value("level",3);
  bool optimize = config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = config_->section("compression").value("buffer_size",getpagesize());
  bool noAuth = config_->value("noauth",false);
  return socket.clientAuth(
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
  );
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
