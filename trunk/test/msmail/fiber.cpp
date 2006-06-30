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
#include <adicpp/adicpp.h>
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
  return server_.config_->section("users").isSection(user);
}
//------------------------------------------------------------------------------
utf8::String ServerFiber::getUserPassword(const utf8::String & user)
{
  return server_.config_->section("users").section(user).text("password");
}
//------------------------------------------------------------------------------
void ServerFiber::auth()
{
  maxSendSize(server_.config_->parse().override().value("max_send_size",getpagesize()));
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
  stdErr.setDebugLevels(server_.config_->parse().override().value("debug_levels","+0,+1,+2,+3"));
  union {
    uint8_t cmd;
    uint8_t ui8;
  };
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
        putCode(ui8 < stCount ? eOK : eInvalidServerType);
        serverType_ = ServerType(ui8);
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
      case cmRemoveMail :
        removeMail();
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
  if( serverType_ != stStandalone && serverType_ != stNode ){
    terminate();
    return;
  }
  utf8::String host(remoteAddress().resolveAsync());
  ServerInfo server(server_.bindAddrs()[0].resolveAsync(defaultPort),stStandalone);
  Server::Data & data = server_.data(serverType_);
  Server::Data tdata, diff;
  tdata.or(data);
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
  bool startNodeClient = false;
  if( serverType_ == stStandalone ){
    AutoMutexWRLock<FiberMutex> lock(data.mutex_);
    diff.xorNL(data,tdata);
    startNodeClient = data.orNL(tdata);
  }
  if( startNodeClient ) server_.startNodeClient(stStandalone);
  putCode(serverType_ == stStandalone ? eOK : eInvalidServerType);
  if( serverType_ == stStandalone ){
    utf8::String::Stream stream;
    stream << serverTypeName_[serverType_] << ": changes stored from client " << host << "\n";
    diff.dumpNL(stream);
    stdErr.debug(5,stream);
// sweep
    stream.clear() << serverTypeName_[serverType_] << ": sweep\n";
    if( data.sweep(
          gettimeofday() -
            (uint64_t) server_.config_->valueByPath(
            utf8::String(serverConfSectionName_[serverType_]) + ".ttl","") * 1000000u,
          &stream)
      ) stdErr.debug(5,stream);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerDB()
{
  utf8::String::Stream stream;
  utf8::String host(remoteAddress().resolveAsync());
  if( serverType_ != stNode ){
    terminate();
    stream << serverTypeName_[serverType_] <<
      ": host " << host << " ask for " << serverTypeName_[serverType_] << " database\n";
    stdErr.debug(6,stream);
    return;
  }
  if( !(bool) server_.config_->parse().override().valueByPath(
        utf8::String(serverConfSectionName_[serverType_]) + ".enabled",false) ){
    terminate();
    stream << serverTypeName_[serverType_] <<
      ": functional disabled, but host " << host << " attempt to register own database.\n";
    stdErr.debug(6,stream);
    return;
  }
  bool dbChanged = false;
  uint64_t ftime = getProcessStartTime(), rftime;
  *this << ftime >> ftime;
  Server::Data rdata, tdata, diff;
  rdata.recvDatabase(*this);
  rftime = ftime;
  stream << serverTypeName_[serverType_] <<
    ": database changes received from host " << host << "\n";
  rdata.dumpNL(stream);
  stdErr.debug(6,stream);
  Server::Data & data = server_.data(serverType_);
  {
    AutoMutexWRLock<FiberMutex> lock(data.mutex_);
    ftime = gettimeofday();
    diff.xorNL(data,rdata);
    tdata.orNL(data,rftime); // get local changes for sending
    dbChanged = data.orNL(rdata); // apply remote changes localy
  }
  tdata.sendDatabaseNL(*this);
  putCode(eOK);
  flush();
//  if( dbChanged ) server_.startNodesExchange();
  stream.clear() << serverTypeName_[serverType_] <<
    ": database changes sended to host " << host << "\n";
  tdata.dumpNL(stream);
  stdErr.debug(6,stream);
  stream.clear() << serverTypeName_[serverType_] << ": changes stored\n";
  diff.dumpNL(stream);
  stdErr.debug(5,stream);
// sweep
  stream.clear() << serverTypeName_[serverType_] << ": sweep\n";
  if( data.sweep(
        gettimeofday() -
          (uint64_t) server_.config_->valueByPath(
          utf8::String(serverConfSectionName_[serverType_]) + ".ttl","") * 1000000u,
        &stream)
    ) stdErr.debug(5,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::getDB()
{
  if( serverType_ != stStandalone && serverType_ != stNode ) return;
  utf8::String host(remoteAddress().resolveAsync());
  Server::Data tdata;
  uint64_t ftime;
  *this >> ftime;
  {
    AutoMutexRDLock<FiberMutex> lock(server_.data(serverType_).mutex_);
    tdata.orNL(server_.data(serverType_),ftime); // get local changes for sending
    ftime = gettimeofday();
  }
  tdata.sendDatabaseNL(*this);
  *this << ftime;
  putCode(eOK);
  flush();
  utf8::String::Stream stream;
  stream << serverTypeName_[serverType_] <<
    ": database sended to client " << host << "\n";
  tdata.dumpNL(stream);
  stdErr.debug(7,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::sendMail() // client sending mail
{
  uintptr_t i;
  AutoPtr<Message> message;
  utf8::String id;
  Stat st;
  bool rest, incomplete = false;

  *this >> id >> rest;
  AsyncFile ctrl, file;
  ctrl.fileName(server_.lckDir() + id + ".msg.lck").
    createIfNotExist(true).removeAfterClose(true).open();
  AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
  file.createIfNotExist(true);
  if( rest ){
    file.fileName(server_.incompleteDir() + id + ".msg");
    statAsync(file.fileName(),st);
    uint64_t remainder;
    *this << uint64_t(st.st_size) >> remainder;
    AutoPtr<uint8_t> b;
    size_t bl = server_.config_->value("max_send_size",getpagesize());
    b.alloc(bl);
    file.open().seek(st.st_size);
    while( remainder > 0 ){
      uint64_t l = remainder > bl ? bl : remainder;
      read(b,l);
      file.write(b,l);
      remainder -= l;
    }
    file.seek(0);
    message = new Message;
    file >> message;
    incomplete = true;
  }
  else {
    message = new Message;
    *this >> message;
  }
  if( !message->isValue("#Recepient") || 
      message->value("#Recepient").trim().strlen() == 0 ||
      !message->isValue(messageIdKey) ||
      id.strcmp(message->id()) != 0 ){
    putCode(eInvalidMessage);
    return;
  }
  utf8::String relay;
  for( i = 0; i < ~uintptr_t(0); i++ ){
    relay = "#Relay." + utf8::int2Str(i);
    if( !message->isValue(relay) ) break;
  }
  message->value(relay,server_.bindAddrs()[0].resolveAsync(defaultPort));
  relay = relay + ".";
  message->value(relay + "Received",getTimeString(gettimeofday()));
  message->value(relay + "Process.Id",utf8::int2Str(ksys::getpid()));
  message->value(relay + "Process.StartTime",getTimeString(getProcessStartTime()));
  if( incomplete ){
    file.seek(0).resize(0) << message;
    file.close();
    renameAsync(file.fileName(),server_.spoolDir() + id + ".msg");
  }
  else {
    file.fileName(server_.spoolDir() + id + ".msg");
    file.removeAfterClose(true).open();
    file << message;
    file.removeAfterClose(false).close();
  }
  putCode(eOK);
  flush();
  stdErr.debug(0,
    utf8::String::Stream() << "Message " << message->id() <<
    " received from " << message->value("#Sender") <<
    " to " << message->value("#Recepient") <<
    ", traffic " <<
    allBytes() << "\n"
  );
}
//------------------------------------------------------------------------------
intptr_t ServerFiber::processMailbox(
  const utf8::String & userMailBox,
  Array<utf8::String> & mids,
  bool onlyNewMail)
{
  stdErr.setDebugLevels(server_.config_->value("debug_levels","+0,+1,+2,+3"));
  utf8::String myHost(server_.bindAddrs()[0].resolveAsync(defaultPort));
  intptr_t i, j, c, k;
  Array<utf8::String> ids;
  Vector<utf8::String> list;
  getDirListAsync(list,userMailBox + "*.msg",utf8::String(),false);
  k = list.count();
  for( i = list.count() - 1; i >= 0; i-- ){
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.createIfNotExist(true).removeAfterClose(true).open();
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
    if( file.isOpen() ){
      utf8::String id(changeFileExt(getNameFromPathName(list[i]),""));
      if( mids.bSearch(id) < 0 || !onlyNewMail ){
        Message message;
        file >> message;
        assert( message.id().strcmp(id) == 0 );
        utf8::String suser, skey;
        message.separateValue("#Recepient",suser,skey);
        bool lostSheep = true;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          Key2ServerLink * key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL )
            lostSheep = key2ServerLink->server_.strcasecmp(myHost) != 0;
        }
        if( lostSheep ){
          file.close();
          renameAsync(file.fileName(),server_.spoolDir() + getNameFromPathName(list[i]));
        }
        else {
          bool messageAccepted = true;
          if( skey.strcasecmp(key_) == 0 && mids.bSearch(message.id()) < 0 ){
            *this << message >> messageAccepted;
            putCode(i > 0 ? eOK : eLastMessage);
            if( onlyNewMail && messageAccepted ){
              j = ids.bSearch(message.id(),c);
              if( c != 0 ) ids.insert(j + (c > 0),message.id());
            }
          }
          file.close();
          k += !messageAccepted;
        }
      }
      k--;
    }
    list.remove(i);
  }
  flush();
  mids = ids;
  return k;
}
//------------------------------------------------------------------------------
void ServerFiber::recvMail() // client receiving mail
{
  utf8::String mailForUser, mailForKey;
  bool waitForMail, onlyNewMail;
  *this >> mailForUser >> mailForKey >> waitForMail >> onlyNewMail;
  user_ = mailForUser;
  key_ = mailForKey;
  utf8::String userMailBox(includeTrailingPathDelimiter(server_.mailDir() + mailForUser));
  createDirectoryAsync(userMailBox);
  putCode(eOK);
  intptr_t k;
  Array<utf8::String> ids;
  server_.addRecvMailFiber(*this);
  try {
    {
// send notify to wait fibers
      UUID uuid;
      createUUID(uuid);
      utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
      AsyncFile watchdog(includeTrailingPathDelimiter(userMailBox) + "." + suuid);
      watchdog.createIfNotExist(true).removeAfterClose(true).open();
    }
    while( !terminated_ ){
      k = processMailbox(userMailBox,ids,onlyNewMail);
      flush();
      if( !waitForMail ) break;
      if( k == 0 ){
        dcn_.monitor(userMailBox);
        stdErr.debug(9,utf8::String::Stream() <<
          this << " Processing mailbox " <<
          excludeTrailingPathDelimiter(userMailBox) <<
          " by monitor... \n"
        );
      }
    }
  }
  catch( ... ){
    server_.remRecvMailFiber(*this);
    throw;
  }
  server_.remRecvMailFiber(*this);
}
//------------------------------------------------------------------------------
void ServerFiber::removeMail() // client remove mail
{
  utf8::String mailForUser, id;
  *this >> mailForUser >> id;
  try {
    utf8::String userMailBox(includeTrailingPathDelimiter(server_.mailDir() + mailForUser));
    createDirectoryAsync(userMailBox);
    {
      AsyncFile ctrl(server_.lckDir() + id + ".msg" + ".lck");
      ctrl.createIfNotExist(true).removeAfterClose(true).open();
      AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
      removeAsync(includeTrailingPathDelimiter(userMailBox) + id + ".msg");
    }
    putCode(eOK);
  }
  catch( ExceptionSP & ){
    putCode(eInvalidMessageId);
    throw;
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
  stdErr.setDebugLevels(server_.config_->parse().override().value("debug_levels","+0,+1,+2,+3"));
  utf8::String myHost(server_.bindAddrs()[0].resolveAsync(defaultPort));
  intptr_t i, k;
  Vector<utf8::String> list;
  getDirListAsync(list,server_.spoolDir() + "*.msg",utf8::String(),false);
  k = list.count();
  while( !terminated_ && list.count() > 0 ){
    i = (intptr_t) server_.rnd_->random(list.count());
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.createIfNotExist(true).removeAfterClose(true).open();
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
            key2ServerLink != NULL && key2ServerLink->server_.strcasecmp(myHost) == 0;
        }
        file.close();
        if( key2ServerLink == NULL ){
          uint64_t messageTTL = server_.config_->valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) +
            ".message_ttl",
            2678400u // 31 day
          );
          uint64_t messageTime = timeFromTimeString(message.value("#Relay.0.Received"));
          if( gettimeofday() - messageTime >= messageTTL ){
            file.close();
            removeAsync(list[i]);
            stdErr.debug(1,utf8::String::Stream() << "Message " <<
              message.id() << "TTL exhausted, removed.\n"
            );
          }
        }
        else {
          if( deliverLocaly ){
            utf8::String userMailBox(server_.mailDir() + suser);
            createDirectoryAsync(userMailBox);
            renameAsync(list[i],includeTrailingPathDelimiter(userMailBox) + message.id() + ".msg");
            stdErr.debug(0,
              utf8::String::Stream() << "Message " << message.id() <<
              " received from " << message.value("#Sender") <<
              " to " << message.value("#Recepient") <<
              " delivered localy to mailbox: " << userMailBox << "\n"
            );
          }
          else {
            renameAsync(list[i],server_.mqueueDir() + message.id() + ".msg");
            stdErr.debug(0,
              utf8::String::Stream() << "Message " << message.id() <<
              " received from " << message.value("#Sender") <<
              " to " << message.value("#Recepient") <<
              " is put in queue for delivery.\n"
            );
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
  while( !terminated_ ){
    k = processQueue();
    if( terminated_ ) break;
    if( k == 0 ){
      dcn_.monitor(excludeTrailingPathDelimiter(server_.spoolDir()));
      stdErr.debug(9,utf8::String::Stream() << this << " Processing spool by monitor... \n");
    }
    else {
      uint64_t timeout = server_.config_->valueByPath(
        utf8::String(serverConfSectionName_[stStandalone]) + ".spool_processing_interval",60u);
      sleepAsync(timeout * 1000000u);
      stdErr.debug(9,utf8::String::Stream() << this << " Processing spool by timer... \n");
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
  maxSendSize(server_.config_->parse().override().value("max_send_size",getpagesize()));
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
intptr_t MailQueueWalker::processQueue(bool & timeWait)
{
  stdErr.setDebugLevels(server_.config_->parse().override().value("debug_levels","+0,+1,+2,+3"));
  intptr_t i, k;
  Vector<utf8::String> list;
  getDirListAsync(list,server_.mqueueDir() + "*.msg",utf8::String(),false);
  k = list.count();
  while( !terminated_ && list.count() > 0 ){
    i = (intptr_t) server_.rnd_->random(list.count());
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.createIfNotExist(true).removeAfterClose(true).open();
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
    if( file.isOpen() ){
      bool resolved = false, connected = false, authentificated = false, sended = false;
      try {
        Message message;
        file >> message;
        utf8::String server, suser, skey;
        message.separateValue("#Recepient",suser,skey);
        ksock::SockAddr address;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          Key2ServerLink * key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL ) server = key2ServerLink->server_;
        }
        if( server.trim().strlen() == 0 ){
          timeWait = true;
          stdErr.debug(1,
            utf8::String::Stream() <<
            "Message recepient " << message.value("#Recepient") <<
            " not found in database.\n"
          );
          server_.startNodeClient(stStandalone);
          uint64_t messageTTL = server_.config_->valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) +
            ".message_ttl",
            2678400u // 31 day
          );
          uint64_t messageTime = timeFromTimeString(message.value("#Relay.0.Received"));
          if( gettimeofday() - messageTime >= messageTTL ){
            file.close();
            removeAsync(list[i]);
            stdErr.debug(1,utf8::String::Stream() << "Message " <<
              list[i] << "TTL exhausted, removed.\n"
            );
          }
        }
        else {
          try {
            address.resolveAsync(server,defaultPort);
            resolved = true;
          }
          catch( ExceptionSP & e ){
            timeWait = true;
            e->writeStdError();
            stdErr.debug(1,
              utf8::String::Stream() <<
                "Unable to resolve " << server <<
                ", message " << message.id() <<
                " recepient " << message.value("#Recepient") << "\n"
            );
          }
          if( resolved ){
            try {
              close();
              connect(address);
              connected = true;
            }
            catch( ExceptionSP & e ){
              timeWait = true;
              e->writeStdError();
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
            catch( ExceptionSP & e ){
              timeWait = true;
              e->writeStdError();
              stdErr.debug(3,
                utf8::String::Stream() << "Authentification to host " <<
                server << " failed.\n"
              );
            }
          }
        }
        if( authentificated ){ // and now we can send message
          sended = false;
          uint64_t restFrom, remainder;
          try {
            *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
            getCode();
            *this << uint8_t(cmSendMail) << message.id() << true >> restFrom;
            file.seek(restFrom);
            *this << (remainder = file.size() - restFrom);
            AutoPtr<uint8_t> b;
            size_t bl = server_.config_->value("max_send_size",getpagesize());
            b.alloc(bl);
            while( remainder > 0 ){
              uint64_t l = remainder > bl ? bl : remainder;
              file.read(b,l);
              write(b,l);
              remainder -= l;
            }
            getCode();
            *this << uint8_t(cmQuit);
            getCode();
            stdErr.debug(0,
              utf8::String::Stream() << "Message " << message.id() <<
              " sended to " << message.value("#Recepient") <<
              " via " << server << ", traffic " <<
              allBytes() << "\n"
            );
            sended = true;
            k--;
          }
          catch( ExceptionSP & e ){
            timeWait = true;
            e->writeStdError();
          }
        }
        shutdown();
        close();
      }
      catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_INVALID_DATA + errorOffset ) throw;
#else
        if( e->code() != EINVAL ) throw;
#endif
        removeAsync(list[i]);
        stdErr.debug(1,utf8::String::Stream() << "Invalid message " << list[i] << " removed.\n");
      }
      if( sended ){
        file.close();
        removeAsync(list[i]);
        stdErr.debug(9,utf8::String::Stream() << "Message " << list[i] << "sended, removed.\n");
      }
    }
    list.remove(i);
  }
  return k;
}
//------------------------------------------------------------------------------
void MailQueueWalker::main()
{
  intptr_t k;
  while( !terminated_ ){
    bool timeWait = false;
    k = processQueue(timeWait);
    if( terminated_ ) break;
    if( k == 0 ){
      dcn_.monitor(excludeTrailingPathDelimiter(server_.mqueueDir()));
      stdErr.debug(9,utf8::String::Stream() << this << " Processing mqueue by monitor... \n");
    }
    else if( timeWait ){
      uint64_t timeout = server_.config_->valueByPath(
        utf8::String(serverConfSectionName_[stStandalone]) + ".mqueue_processing_interval",60u);
      sleepAsync(timeout * 1000000u);
      stdErr.debug(9,utf8::String::Stream() << this << " Processing mqueue by timer... \n");
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
NodeClient::NodeClient(
  Server & server,
  ServerType dataType,
  const utf8::String & nodeHostName,
  bool periodicaly) :
  server_(server),
  dataType_(dataType),
  nodeHostName_(nodeHostName),
  periodicaly_(periodicaly)
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
  maxSendSize(server_.config_->parse().override().value("max_send_size",getpagesize()));
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
void NodeClient::main()
{
  stdErr.setDebugLevels(server_.config_->parse().override().value("debug_levels","+0,+1,+2,+3"));
  intptr_t i;
  utf8::String server, host;
  try {
    bool connected, exchanged, doWork;
    do {
      connected = exchanged = false;
      {
        AutoLock<FiberInterlockedMutex> lock(server_.nodeClientMutex_);
        doWork = 
          dataType_ != stStandalone ||
          (server_.nodeClient_ == NULL && periodicaly_) ||
          server_.nodeClient_ == this
        ;
      }
      if( doWork ){
        if( dataType_ == stStandalone ){
          server = server_.data(stStandalone).getNodeList();
          host = server_.data(stNode).getNodeList();
          if( server.strlen() > 0 && host.strlen() > 0 ) server += ",";
          server += host;
          host = server_.config_->parse().override().valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) + ".node",""
          );
          if( server.strlen() > 0 && host.strlen() > 0 ) server += ",";
          server += host;
        }
        else {
          server = nodeHostName_;
        }
        i = enumStringParts(server);
        while( !exchanged && !terminated_ && --i >= 0 ){
          try {
            ksock::SockAddr remoteAddress;
            remoteAddress.resolveAsync(stringPartByNo(server,i),defaultPort);
            host = remoteAddress.resolveAsync();
            close();
            connect(remoteAddress);
            try {
              auth();
              connected = true;
            }
            catch( ExceptionSP & e ){
              e->writeStdError();
              stdErr.debug(3,utf8::String::Stream() <<
                "Authentification to host " << host << " failed.\n"
              );
            }
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
            stdErr.debug(3,utf8::String::Stream() <<
              "Unable connect to node. Host " << host << " unreachable.\n"
            );
          }
          if( connected ){
            try {
              utf8::String::Stream stream;
              *this << uint8_t(cmSelectServerType) << uint8_t(stNode);
              getCode();
              Server::Data & data = server_.data(dataType_);
              data.registerServer(ServerInfo(host,stNode));
              Server::Data tdata, ldata, diff, dump;
              uint64_t rStartTime, ftime = 0, rftime;
              *this << uint8_t(cmRegisterDB) >> rStartTime;
              bool fullDump = false;
              {
                AutoMutexRDLock<FiberMutex> lock(data.mutex_);
                ServerInfo * si = data.servers_.find(host);
                if( si != NULL ){
                  fullDump = si->stime_ != rStartTime;
                  ftime = si->ftime_;
                }
                dump.orNL(data);
                ldata.orNL(data,fullDump ? 0 : ftime);
                data.mtime_ = ~uint64_t(0);
              }
              *this << ftime;
              ldata.sendDatabaseNL(*this);
              tdata.recvDatabaseNL(*this);
              getCode();
              {
                AutoMutexWRLock<FiberMutex> lock(data.mutex_);
                rftime = gettimeofday();
                if( data.mtime_ < ~uint64_t(0) && data.mtime_ >= ftime )
                  rftime = data.mtime_ - 1;
                data.mtime_ = 0;
                data.orNL(tdata,0,rftime);
                ServerInfo * si = data.servers_.find(host);
                if( si != NULL ){
                  si->ftime_ = rftime;
                  if( fullDump ) si->stime_ = rStartTime;
                }
              }
              exchanged = true;
              *this << uint8_t(cmQuit);
              getCode();
              stream.clear() << "NODE client: " << serverTypeName_[dataType_] <<
                " database " << (fullDump ? "full dump" : "changes") <<
                " sended to node " << host << "\n";
              ldata.dumpNL(stream);
              stdErr.debug(6,stream);
              stream.clear() << "NODE client: " << serverTypeName_[dataType_] <<
                " database changes received from node " << host << "\n";
              tdata.dumpNL(stream);
              stdErr.debug(6,stream);
              diff.xorNL(dump,tdata);
              stream.clear() << "NODE client: changes stored in " <<
                serverTypeName_[dataType_] << " database.\n";
              diff.dumpNL(stream);
              stdErr.debug(5,stream);
            }
            catch( ExceptionSP & e ){
              e->writeStdError();
            }
          }
        }
      }
      if( periodicaly_ ){
        uint64_t timeout = server_.config_->parse().override().
          valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) + ".exchange_interval",
            60u
          );
        sleepAsync(timeout * 1000000u);
      }
    } while( periodicaly_ && !terminated_ );
    AutoLock<FiberInterlockedMutex> lock(server_.nodeClientMutex_);
    if( !periodicaly_ && !exchanged ){
      if( dataType_ == stStandalone ) server_.skippedNodeClientStarts_++;
      if( dataType_ == stNode ) server_.skippedNodeExchangeStarts_++;
    }
  }
  catch( ... ){
    if( !periodicaly_ ) server_.clearNodeClient(this);
    throw;
  }
  if( !periodicaly_ ) server_.clearNodeClient(this);
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
