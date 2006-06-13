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
//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "msmail.h"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ServerFiber::~ServerFiber()
{
}
//------------------------------------------------------------------------------
ServerFiber::ServerFiber(Server & server) : server_(server), serverType_(stNone)
{
}
//------------------------------------------------------------------------------
void ServerFiber::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    throw ExceptionSP(new Exception(code,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void ServerFiber::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
void ServerFiber::putCode(int32_t code)
{
  *this << int32_t(code);
}
//------------------------------------------------------------------------------
bool ServerFiber::isValidUser(const utf8::String & user)
{
  return server_.config_->parse().override().section("users").isSection(user);
}
//------------------------------------------------------------------------------
utf8::String ServerFiber::getUserPassword(const utf8::String & user)
{
  return server_.config_->section("users").section(user).text("password");
}
//------------------------------------------------------------------------------
void ServerFiber::auth()
{
  maxSendSize(server_.config_->value("max_send_size",getpagesize()));
  utf8::String encryption(server_.config_->section("encryption").text(utf8::String(),"default"));
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  utf8::String compression(server_.config_->section("compression").text(utf8::String(),"default"));
  utf8::String compressionType(server_.config_->section("compression").text("type","default"));
  utf8::String crc(server_.config_->section("compression").text("crc","default"));
  uintptr_t compressionLevel = server_.config_->section("compression").value("max_level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
  Error e = (Error) serverAuth(
    encryption,
    encryptionThreshold,
    compression,
    compressionType,
    crc,
    compressionLevel,
    optimize,
    bufferSize
  );
  if( e != eOK ) throw ExceptionSP(new Exception(e,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void ServerFiber::main()
{
  union {
    uint8_t cmd;
    uint8_t ui8;
  };
  //FindFirstChangeNotification must be used ander Win9x
  //ReadDirectoryChangesW must be used ander Win2000, XP ...
  auth();
  while( !terminated_ ){
    *this >> cmd;
    switch( cmd ){
      case cmQuit :
        putCode(eOK);
        terminate();
        break;
      case cmGetProcessStartTime :
        *this << getProcessStartTime();
        break;
      case cmSelectServerType :
        *this >> ui8;
        serverType_ = ServerType(ui8);
        putCode(serverType_ < stCount ? eOK : eInvalidServerType);
        break;
      case cmRegisterClient :
        registerClient();
        break;
      case cmRegisterDB :
        registerDB();
        break;
      case cmGetDB :
        getDB();
        break;
      case cmSendMail :
        sendMail();
        break;
      case cmRecvMail :
        recvMail();
        break;
      default : // unrecognized or unsupported command, terminate
        putCode(eInvalidCommand);
        terminate();
    }
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerClient()
{
  Server::Data & data = server_.data(serverType_);
  Server::Data tdata;
  ServerInfo server(ksock::SockAddr::gethostname());
  tdata.registerServerNL(server);
  UserInfo user;
  *this >> user;
  tdata.registerUserNL(user);
  KeyInfo key;
  *this >> key;
  tdata.registerKeyNL(key);
  tdata.registerUser2KeyLinkNL(User2KeyLink(user.name_,key.name_));
  tdata.registerKey2ServerLinkNL(Key2ServerLink(key.name_,server.name_));
  uint64_t u;
  *this >> u;
  while( u-- > 0 ){
    GroupInfo group;
    *this >> group;
    tdata.registerGroupNL(group);
    tdata.registerKey2GroupLinkNL(Key2GroupLink(key.name_,group.name_));
  }
  {
    AutoMutexWRLock<FiberMutex> lock(data.mutex_);
    data.intersectionNL(tdata);
  }
  putCode(eOK);
}
//------------------------------------------------------------------------------
void ServerFiber::registerDB()
{
  Server::Data tdata;
  tdata.recvDatabase(*this);
  Server::Data & data = server_.data(serverType_);
  {
    AutoMutexRDLock<FiberMutex> lock(data.mutex_);
    data.intersectionNL(tdata);
    data.ftime_ = gettimeofday();
  }
  putCode(eOK);
}
//------------------------------------------------------------------------------
void ServerFiber::getDB()
{
  Server::Data & data = server_.data(serverType_);
  Server::Data tdata;
  {
    AutoMutexRDLock<FiberMutex> lock(data.mutex_);
    tdata.intersectionNL(data);
    tdata.ftime_ = data.ftime_;
  }
  tdata.sendDatabase(*this);
  putCode(eOK);
}
//------------------------------------------------------------------------------
void ServerFiber::sendMail() // client sending mail
{
  uintptr_t i;

  AutoPtr<Message> message(new Message);
  *this >> message;
  if( !message->isValue("#Recepient") || message->value("#Recepient").trim().strlen() == 0 ){
    putCode(eInvalidMessage);
  }
  else {
    if( !message->isValue(messageIdKey) || message->value(messageIdKey).trim().strlen() == 0 ){
      UUID uuid;
      createUUID(uuid);
      utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
      message->value(messageIdKey,suuid);
    }
    utf8::String relay;
    for( i = 0; i < ~uintptr_t(0); i++ ){
      relay = "#Relay." + utf8::int2Str(i);
      if( !message->isValue(relay) ) break;
    }
    message->value(relay,ksock::SockAddr::gethostname());
    relay = relay + ".";
    message->value(relay + "Received",getTimeString(gettimeofday()));
    message->value(relay + "Process.Id",utf8::int2Str(ksys::getpid()));
    message->value(relay + "Process.StartTime",getTimeString(getProcessStartTime()));
    utf8::String spool(server_.spoolDir());
    AsyncFile file(spool + message->value(messageIdKey) + ".msg");
    file.removeAfterClose(true).exclusive(true).open();
    file << message;
    file.removeAfterClose(false).close();
    putCode(eOK);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::recvMail() // client receiving mail
{
  utf8::String mailForUser, mailForKey;
  uint8_t waitForMail, onlyNewMail;
  *this >> mailForUser >> mailForKey >> waitForMail >> onlyNewMail;
  utf8::String userMailBox(includeTrailingPathDelimiter(server_.mailDir() + mailForUser));
  createDirectoryAsync(userMailBox);
  DirectoryChangeNotification dcn;
  putCode(eOK);
  Array<utf8::String> ids;
  while( !terminated_ ){
    Vector<utf8::String> list;
    getDirListAsync(list,userMailBox + "*.msg",utf8::String(),false);
    for( intptr_t i = list.count() - 1; i >= 0; i-- ){
      AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
      ctrl.removeAfterClose(true);
      ctrl.open();
      AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
      AsyncFile file(list[i]);
      file.open();
      Message message;
      file >> message;
      if( !onlyNewMail || ids.bSearch(message.id()) < 0 ){
        stdErr.debug(1,utf8::String::Stream() << "Processing message " << message.id() << " in recvMail.\n");
        *this << message;
      }
      file.close();
      putCode(i > 0 ? eOK : eLastMessage);
      if( onlyNewMail ){
        intptr_t c, j = ids.bSearch(message.id(),c);
        if( c != 0 ) ids.insert(j + (c > 0),message.id());
      }
      list.remove(i);
    }
    if( waitForMail ){
      dcn.monitor(userMailBox);
    }
    else {
      terminate();
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SpoolWalker::~SpoolWalker()
{
}
//------------------------------------------------------------------------------
SpoolWalker::SpoolWalker(Server & server) : server_(server)
{
}
//------------------------------------------------------------------------------
intptr_t SpoolWalker::processQueue()
{
  intptr_t i, k;
  Vector<utf8::String> list;
  getDirListAsync(list,server_.spoolDir() + "*.msg",utf8::String(),false);
  k = list.count();
  while( !terminated_ && list.count() > 0 ){
    i = (intptr_t) server_.rnd_->random(list.count());
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.removeAfterClose(true);
    ctrl.open();
    AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
    AsyncFile file(list[i]);
    try {
      file.open();
    }
    catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
          e->code() != ERROR_FILE_NOT_FOUND + errorOffset &&
          e->code() != ERROR_ACCESS_DENIED + errorOffset ) throw;
      e->writeStdError();
#else
#error Not implemented
#endif
    }
    try {
      if( file.isOpen() ){
        Message message;
        file >> message;
        utf8::String suser, skey;
        message.separateValue("#Recepient",suser,skey);
        bool deliverLocaly;
        Key2ServerLink * key2ServerLink = NULL;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          key2ServerLink = data.key2ServerLinks_.find(skey);
          deliverLocaly = 
            key2ServerLink != NULL && 
            key2ServerLink->server_.strcasecmp(ksock::SockAddr::gethostname()) == 0;
        }
        file.close();
        if( key2ServerLink != NULL ){
          if( deliverLocaly ){
            utf8::String userMailBox(server_.mailDir() + suser);
            createDirectoryAsync(userMailBox);
            renameAsync(list[i],includeTrailingPathDelimiter(userMailBox) + message.id() + ".msg");
          }
          else {
            renameAsync(list[i],server_.mqueueDir() + message.id() + ".msg");
          }
          k--;
        }
      }
    }
    catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_INVALID_DATA + errorOffset ) throw;
#else
      if( e->code() != EINVAL ) throw;
#endif
      stdErr.debug(1,utf8::String::Stream() << "Invalid message " << list[i] << "\n");
    }
    list.remove(i);
  }
  return k;
}
//------------------------------------------------------------------------------
void SpoolWalker::fiberExecute()
{
  intptr_t k;
  DirectoryChangeNotification dcn;
  while( !terminated_ ){
    k = processQueue();
    if( terminated_ ) break;
    if( k == 0 ){
      dcn.monitor(excludeTrailingPathDelimiter(server_.spoolDir()));
      stdErr.debug(1,utf8::String::Stream() << "Processing spool by monitor... \n");
    }
    else {
      try {
        uint64_t timeout = server_.config_->value("queue_processing_interval",10000000u);
        sleepAsync(timeout);
        stdErr.debug(1,utf8::String::Stream() << "Processing spool by timer... \n");
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_REQUEST_ABORTED + errorOffset ) throw;
#else
        if( e->code() != EINTR ) throw;
#endif
      }
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MailQueueWalker::~MailQueueWalker()
{
}
//------------------------------------------------------------------------------
MailQueueWalker::MailQueueWalker(Server & server) : server_(server)
{
}
//------------------------------------------------------------------------------
void MailQueueWalker::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    throw ksys::ExceptionSP(new ksys::Exception(code,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void MailQueueWalker::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
void MailQueueWalker::auth()
{
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(server_.config_->value("max_send_size",getpagesize()));
  user = server_.config_->text("user","system");
  password = server_.config_->text("password","sha256:D7h+DiEkmuy6kSKdj9YoFurRn2Cbqoa2qGdd5kocOjE");
  encryption = server_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = server_.config_->section("compression").text(utf8::String(),"default");
  compressionType = server_.config_->section("compression").value("type","default");
  crc = server_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = server_.config_->section("compression").value("level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
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
intptr_t MailQueueWalker::processQueue()
{
  intptr_t i, k;
  Vector<utf8::String> list;
  getDirListAsync(list,server_.mqueueDir() + "*.msg",utf8::String(),false);
  k = list.count();
  while( !terminated_ && list.count() > 0 ){
    i = (intptr_t) server_.rnd_->random(list.count());
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.removeAfterClose(true);
    ctrl.open();
    AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
    AsyncFile file(list[i]);
    try {
      file.open();
    }
    catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
          e->code() != ERROR_FILE_NOT_FOUND + errorOffset &&
          e->code() != ERROR_ACCESS_DENIED + errorOffset ) throw;
      if( e->code() == ERROR_ACCESS_DENIED ) e->writeStdError();
#else
#error Not implemented
#endif
    }
    try {
      if( file.isOpen() ){
        Message message;
        file >> message;
        file.close();
        utf8::String server, suser, skey;
        message.separateValue("#Recepient",suser,skey);
        bool resolved = false, connected = false, authentificated = false, sended = false;
        ksock::SockAddr address;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          Key2ServerLink * key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL ) server = key2ServerLink->server_;
        }
        if( server.trim().strlen() == 0 ){
          stdErr.debug(1,
            utf8::String::Stream() <<
            "Message recepient " << message.value("#Recepient") <<
            " not found in database.\n"
          );
          server_.startNodeClient();
        }
        else {
          try {
            address.resolveAsync(server);
            resolved = true;
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
            stdErr.debug(1,
              utf8::String::Stream() <<
                "Invalid message " << message.id() <<
                " recepient " << message.value("#Recepient") << "\n"
            );
          }
          if( resolved ){
            try {
              connect(address);
              connected = true;
            }
            catch( ExceptionSP & ){
              stdErr.debug(3,
                utf8::String::Stream() <<
                "Unable to connect. Host " << server << " unreachable.\n"
              );
            }
          }
          if( connected ){
            try {
              auth();
              authentificated = true;
            }
            catch( ExceptionSP & ){
              stdErr.debug(3,
                utf8::String::Stream() << "Authentification to host " <<
                server << " failed.\n"
              );
            }
          }
          if( authentificated ){ // and now we can send message
            sended = false;
            try {
              *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
              getCode();
              *this << uint8_t(cmSendMail) << message;
              getCode();
              stdErr.debug(0,utf8::String::Stream() <<
                "Message " << message.id() <<
                " sended to " << message.value("#Recepient") <<
                ", traffic " << allBytes() << "\n"
              );
              sended = true;
              k--;
            }
            catch( ExceptionSP & e ){
              e->writeStdError();
            }
            shutdown();
            close();
          }
        }
      }
    }
    catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_INVALID_DATA + errorOffset ) throw;
#else
      if( e->code() != EINVAL ) throw;
#endif
      stdErr.debug(1,utf8::String::Stream() << "Invalid message " << list[i] << "\n");
    }
    list.remove(i);
  }
  return k;
}
//------------------------------------------------------------------------------
void MailQueueWalker::main()
{
  intptr_t k;
  DirectoryChangeNotification dcn;
  while( !terminated_ ){
    k = processQueue();
    if( terminated_ ) break;
    if( k == 0 ){
      dcn.monitor(excludeTrailingPathDelimiter(server_.mqueueDir()));
      stdErr.debug(1,utf8::String::Stream() << "Processing mqueue by monitor... \n");
    }
    else {
      try {
        uint64_t timeout = server_.config_->value("mqueue_processing_interval",10000000u);
        sleepAsync(timeout);
        stdErr.debug(1,utf8::String::Stream() << "Processing mqueue by timer... \n");
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_REQUEST_ABORTED + errorOffset ) throw;
#else
        if( e->code() != EINTR ) throw;
#endif
      }
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
NodeClient::~NodeClient()
{
}
//------------------------------------------------------------------------------
NodeClient::NodeClient(Server & server) : server_(server)
{
}
//------------------------------------------------------------------------------
void NodeClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    throw ksys::ExceptionSP(new ksys::Exception(code,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void NodeClient::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
void NodeClient::auth()
{
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(server_.config_->value("max_send_size",getpagesize()));
  user = server_.config_->text("user","system");
  password = server_.config_->text("password","sha256:D7h+DiEkmuy6kSKdj9YoFurRn2Cbqoa2qGdd5kocOjE");
  encryption = server_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = server_.config_->section("compression").text(utf8::String(),"default");
  compressionType = server_.config_->section("compression").value("type","default");
  crc = server_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = server_.config_->section("compression").value("level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
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
void NodeClient::clearNodeClient()
{
  AutoLock<FiberInterlockedMutex> lock(server_.nodeClientMutex_);
  server_.nodeClient_ = NULL;
}
//------------------------------------------------------------------------------
void NodeClient::main()
{
  try {
    intptr_t i;
    bool connected = false;
    utf8::String server(server_.config_->value("node",""));
    for( i = enumStringParts(server) - 1; i >= 0 && !terminated_; i-- ){
      ksock::SockAddr remoteAddress;
      remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
      try {
        open();
        connect(remoteAddress);
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        stdErr.debug(3,
          utf8::String::Stream() << "Unable connect to node. Host " <<
          stringPartByNo(server,i) <<
          " unreachable.\n"
        );
      }
      try {
        auth();
        connected = true;
        i = 0;
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        stdErr.debug(3,
          utf8::String::Stream() << "Authentification to host " <<
          stringPartByNo(server,i) << " failed.\n"
        );
      }
    }
    if( connected ){
      *this << uint8_t(cmSelectServerType) << uint8_t(stNode);
      getCode();
      Server::Data & data = server_.data(stStandalone);
      Server::Data tdata;
      {
        AutoMutexRDLock<FiberMutex> lock(data.mutex_);
        tdata.intersectionNL(data);
        tdata.ftime_ = data.ftime_;
      }
      *this << uint8_t(cmRegisterDB);
      tdata.sendDatabase(*this);
      getCode();
      stdErr.debug(2,utf8::String::Stream() <<
        "Database registered on node " << stringPartByNo(server,i) << " succefully.\n"
      );
      *this << uint8_t(cmGetDB);
      tdata.recvDatabase(*this);
      getCode();
      stdErr.debug(2,utf8::String::Stream() <<
        "Database received from node " << stringPartByNo(server,i) << " succefully.\n"
      );
      AutoMutexWRLock<FiberMutex> lock(data.mutex_);
      data.intersectionNL(tdata);
      if( data.ftime_ == tdata.ftime_ ) data.ftime_ = gettimeofday();
    }
  }
  catch( ... ){
    clearNodeClient();
    throw;
  }
  clearNodeClient();
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
