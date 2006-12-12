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
ServerFiber::ServerFiber(Server & server,utf8::String user,utf8::String key) :
  server_(server), serverType_(stNone), protocol_(0), user_(user), key_(key)
{
}
//------------------------------------------------------------------------------
void ServerFiber::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  maxRecvSize(server_.config_->parse().override().value("max_recv_size",-1));
  maxSendSize(server_.config_->value("max_send_size",-1));
  utf8::String encryption(server_.config_->section("encryption").text(utf8::String(),"default"));
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  utf8::String compression(server_.config_->section("compression").text(utf8::String(),"default"));
  utf8::String compressionType(server_.config_->section("compression").text("type","default"));
  utf8::String crc(server_.config_->section("compression").text("crc","default"));
  uintptr_t compressionLevel = server_.config_->section("compression").value("max_level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
  bool noAuth = false;
  if( server_.config_->isSection("users") )
    noAuth = server_.config_->section("users").value("noauth",noAuth);
  Error e = (Error) serverAuth(
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
  if( e != eOK ) newObject<Exception>(e,__PRETTY_FUNCTION__)->throwSP();
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
      case cmSelectProtocol :
        *this >> ui8;
        putCode(ui8 < 2 ? eOK : eInvalidProtocol);
        protocol_ = ui8;
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
  utf8::String host(remoteAddress().resolve(~uintptr_t(0)));
  ServerInfo server(server_.bindAddrs()[0].resolve(defaultPort),stStandalone);
  Server::Data & data = server_.data(serverType_);
  Server::Data tdata, diff;
  tdata.ore(data);
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
  flush();
  if( serverType_ == stStandalone ){
    utf8::String::Stream stream;
    stream << serverTypeName_[serverType_] << ": changes stored from client " << host << "\n";
    diff.dumpNL(stream);
    stdErr.debug(5,stream);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerDB()
{
  utf8::String::Stream stream;
  utf8::String host(remoteAddress().resolve(~uintptr_t(0))), server, service;
  if( serverType_ != stNode ){
    terminate();
    stream << serverTypeName_[serverType_] <<
      ": " << host << " ask for " << serverTypeName_[serverType_] << " database\n";
    stdErr.debug(6,stream);
    return;
  }
  if( !(bool) server_.config_->parse().override().valueByPath(
        utf8::String(serverConfSectionName_[serverType_]) + ".enabled",false) ){
    terminate();
    stream << serverTypeName_[serverType_] <<
      ": functional disabled, but " << host << " attempt to register own database.\n";
    stdErr.debug(6,stream);
    return;
  }
  bool irsl = server_.config_->valueByPath(
    utf8::String(serverConfSectionName_[serverType_]) + ".ignore_remote_send_list",false
  );
  uint32_t port;
  uint64_t rStartTime;
  bool dbChanged = false;
  *this >> port >> rStartTime << uint64_t(getProcessStartTime());
  uint8_t rdbt;
  if( protocol_ > 0 ){
    *this >> rdbt;
    if( rdbt >= stCount ){
      terminate();
      stream << serverTypeName_[serverType_] <<
        ": remote host " << host << " send invalid own database type.\n";
      stdErr.debug(6,stream);
      return;
    }
  }
  splitString(host,server,service,":");
  host = server;
  if( port != defaultPort ) host += ":" + utf8::int2Str(port);
  utf8::String hostDB(host);
  if( protocol_ > 0 ) hostDB = hostDB + " " + serverTypeName_[rdbt];
  Server::Data rdata, ldata, diff;
  rdata.recvDatabaseNL(*this);
  stream << serverTypeName_[serverType_] <<
    ": database changes received from " << hostDB << "\n";
  rdata.dumpNL(stream);
  stdErr.debug(6,stream);
  Server::Data & data = server_.data(serverType_);
  bool fullDump;
  {
    AutoMutexWRLock<FiberMutex> lock(data.mutex_);
    ServerInfo * si = data.servers_.find(host);
    fullDump = si == NULL || si->stime_ != rStartTime;
    diff.xorNL(data,rdata);
    if( irsl /*|| fullDump*/ /*rStartTime < getProcessStartTime()*/ ){
      rdata.clearSendedToNL();
    }
    rdata.setSendedToNL(hostDB);
    dbChanged = data.orNL(rdata); // apply remote changes localy
    ldata.orNL(data,fullDump ? utf8::String() : hostDB); // get local changes for sending
    ldata.setSendedToNL(hostDB);
    data.orNL(ldata);
    si = data.servers_.find(host);
    if( si != NULL && fullDump ) si->stime_ = rStartTime;
  }
  ldata.sendDatabaseNL(*this);
  putCode(eOK);
  flush();
  if( dbChanged ) server_.startNodesExchange();
  stream.clear() << serverTypeName_[serverType_] <<
    ": database " << (fullDump ? "full dump" : "changes") << " sended to " << hostDB << "\n";
  ldata.dumpNL(stream);
  stdErr.debug(fullDump ? 7 : 6,stream);
  stream.clear() << serverTypeName_[serverType_] << ": changes stored\n";
  diff.dumpNL(stream);
  stdErr.debug(5,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::getDB()
{
  if( serverType_ != stStandalone && serverType_ != stNode ) return;
  utf8::String host(remoteAddress().resolve(defaultPort));
  Server::Data tdata;
  tdata.ore(server_.data(serverType_));
  tdata.sendDatabaseNL(*this);
  putCode(eOK);
  flush();
  utf8::String::Stream stream;
  stream << serverTypeName_[serverType_] <<
    ": database sended to client " << host << "\n";
  tdata.dumpNL(stream);
  stdErr.debug(8,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::sendMail() // client sending mail
{
  uintptr_t i;
  AutoPtr<Message> message;
  utf8::String id;
  Stat st;
  bool rest, incomplete = false;
  uint64_t rb = recvBytes(), sb = sendBytes();

  *this >> id >> rest;
  AsyncFile file;
  file.fileName(server_.incompleteDir() + id + ".msg").createIfNotExist(true);
  if( rest ){
    stat(file.fileName(),st);
    uint64_t remainder;
    *this << uint64_t(st.st_size) >> remainder;
    AutoPtr<uint8_t> b;
    size_t bl = getpagesize() * 16;
    b.alloc(bl);
    file.open().seek(st.st_size);
    while( remainder > 0 ){
      uint64_t l = remainder > bl ? bl : remainder;
      read(b,l);
      file.write(b,l);
      remainder -= l;
    }
    file.seek(0);
    message = newObject<Message>();
    file >> message;
    incomplete = true;
  }
  else {
    message = newObject<Message>();
    message->file().fileName(file.fileName()).createIfNotExist(true).removeAfterClose(true);
    *this >> message;
  }
  utf8::String::Stream stream;
  stream << "Message " << message->id() <<
    " received from " << message->value("#Sender") <<
    " to " << message->value("#Recepient") <<
    ", traffic " <<
    recvBytes() - rb + sendBytes() - sb << "\n"
  ;
  if( !message->isValue("#Recepient") || 
      message->value("#Recepient").trim().strlen() == 0 ||
      !message->isValue(messageIdKey) ||
      id.strcmp(message->id()) != 0 ){
    file.removeAfterClose(true);
    putCode(eInvalidMessage);
    return;
  }
  utf8::String relay;
  for( i = 0; i < ~uintptr_t(0); i++ ){
    relay = "#Relay." + utf8::int2Str(i);
    if( !message->isValue(relay) ) break;
  }
  message->value(relay,server_.bindAddrs()[0].resolve(defaultPort));
  relay = relay + ".";
  message->value(relay + "Received",getTimeString(gettimeofday()));
  message->value(relay + "Process.Id",utf8::int2Str(ksys::getpid()));
  message->value(relay + "Process.StartTime",getTimeString(getProcessStartTime()));
  AsyncFile file2(file.fileName() + ".tmp");
  file2.createIfNotExist(true).removeAfterClose(true).open();
  file2 << message;
  file2.removeAfterClose(false).close();
  message = NULL;
  try {
    rename(
      file2.fileName(),
      server_.spoolDir(id.hash(true) & (server_.spoolFibers_ - 1)) + id + ".msg"
    );
    file.removeAfterClose(true).close();
    putCode(eOK);
    flush();
    stdErr.debug(0,stream);
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    try {
      remove(file2.fileName());
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
  }
}
//------------------------------------------------------------------------------
void ServerFiber::processMailbox(
  const utf8::String & userMailBox,
  Message::Keys & mids,
  bool onlyNewMail,
  bool & wait)
{
  wait = true;
  stdErr.setDebugLevels(server_.config_->value("debug_levels","+0,+1,+2,+3"));
  utf8::String myHost(server_.bindAddrs()[0].resolve(defaultPort));
  intptr_t i;
  Message::Keys ids;
  AutoHashDrop<Message::Keys> idsAutoDrop(ids);
  Vector<utf8::String> list;
  getDirList(list,userMailBox + "*.msg",utf8::String(),false);
  while( !terminated_ && list.count() > 0 ){
    i = (intptr_t) server_.rnd_->random(list.count());
    AsyncFile ctrl(server_.lckDir() + getNameFromPathName(list[i]) + ".lck");
    ctrl.createIfNotExist(true).removeAfterClose(true).open();
    AutoFileWRLock<AsyncFile> flock(ctrl);
    AsyncFile file(list[i]);
    try {
      file.open();
      if( file.size() == 0 ) file.removeAfterClose(true).close();
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      wait = true;
    }
    if( file.isOpen() ){
      utf8::String id(changeFileExt(getNameFromPathName(list[i]),""));
      if( mids.find(id) == NULL || !onlyNewMail ){
        Message message;
        file >> message;
        assert( message.id().strcmp(id) == 0 );
        utf8::String suser, skey;
        splitString(message.value("#Recepient"),suser,skey,"@");
        bool lostSheep = true;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          Key2ServerLink * key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL )
            lostSheep = key2ServerLink->server_.strcasecmp(myHost) != 0;
        }
        lostSheep = user_.strcasecmp(suser) != 0 || lostSheep;
        if( lostSheep ){
          file.close();
          rename(file.fileName(),server_.spoolDir(id.hash(true) & (server_.spoolFibers_ - 1)) + getNameFromPathName(list[i]));
        }
        else {
          bool messageAccepted = true;
          if( skey.strcasecmp(key_) == 0 && mids.find(message.id()) == NULL ){
            if( message.isValue("#request.user.online") && message.value("#request.user.online").strlen() == 0 ){
              AutoLock<FiberInterlockedMutex> lock(server_.recvMailFibersMutex_);
              ServerFiber * fib = server_.findRecvMailFiberNL(*this);
              if( fib == NULL ){
                server_.sendRobotMessage(
                  message.value("#Sender"),
                  message.value("#Recepient"),
                  message.value("#Sender.Sended"),
                  "#request.user.online","no"
                );
                if( (bool) Mutant(message.isValue("#request.user.remove.message.if.offline")) ){
                  file.close();
                  remove(list[i]);
                  stdErr.debug(1,
                    utf8::String::Stream() << "Message " << message.id() <<
                    " received from " << message.value("#Sender") <<
                    " to " << message.value("#Recepient") <<
                    " removed, because '#request.user.online' == 'no' and '#request.user.remove.message.if.offline' == 'yes' \n"
                  );
                }
              }
            }
            *this << message >> messageAccepted;
            putCode(i > 0 ? eOK : eLastMessage);
            if( onlyNewMail && messageAccepted ){
              ids.insert(*newObject<Message::Key>(message.id()),false);
	          }
          }
          file.close();
          if( wait && !messageAccepted ) wait = false;
        }
      }
    }
    list.remove(i);
  }
  flush();
  mids = ids;
}
//------------------------------------------------------------------------------
void ServerFiber::recvMail() // client receiving mail
{
  utf8::String mailForUser, mailForKey;
  bool waitForMail, onlyNewMail, wait;
  *this >> mailForUser >> mailForKey >> waitForMail >> onlyNewMail;
  user_ = mailForUser;
  key_ = mailForKey;
  utf8::String userMailBox(includeTrailingPathDelimiter(server_.mailDir() + mailForUser));
  createDirectory(excludeTrailingPathDelimiter(userMailBox));
  putCode(eOK);
  Message::Keys ids;
  AutoHashDrop<Message::Keys> idsAutoDrop(ids);
  server_.addRecvMailFiber(*this);
  try {
    server_.sendUserWatchdog(mailForUser);
    while( !terminated_ ){
      processMailbox(userMailBox,ids,onlyNewMail,wait);
      if( !waitForMail ) break;
      if( wait ){
        dcn_.monitor(userMailBox);
        stdErr.debug(9,utf8::String::Stream() <<
          "Processing mailbox " <<
          excludeTrailingPathDelimiter(userMailBox) <<
          " by monitor... \n"
        );
      }
    }
  }
  catch( ... ){
    server_.remRecvMailFiber(*this);
//    processMailbox(userMailBox,ids,onlyNewMail,wait);
    throw;
  }
  server_.remRecvMailFiber(*this);
//  processMailbox(userMailBox,ids,onlyNewMail,wait);
}
//------------------------------------------------------------------------------
void ServerFiber::removeMail() // client remove mail
{
  utf8::String mailForUser, id;
  *this >> mailForUser >> id;
  try {
    utf8::String userMailBox(includeTrailingPathDelimiter(server_.mailDir() + mailForUser));
    createDirectory(userMailBox);
    {
      AsyncFile ctrl(server_.lckDir() + id + ".msg" + ".lck");
      ctrl.createIfNotExist(true).removeAfterClose(true).open();
      AutoFileWRLock<AsyncFile> flock(ctrl,0,0);
      remove(includeTrailingPathDelimiter(userMailBox) + id + ".msg");
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
SpoolWalker::SpoolWalker(Server & server,intptr_t id) : server_(server), id_(id)
{
}
//------------------------------------------------------------------------------
void SpoolWalker::processQueue(bool & timeWait)
{
  timeWait = false;
  stdErr.setDebugLevels(server_.config_->parse().override().value("debug_levels","+0,+1,+2,+3"));
  utf8::String myHost(server_.bindAddrs()[0].resolve(defaultPort));
  Vector<utf8::String> list;
  getDirList(list,server_.spoolDir(id_) + "*.msg",utf8::String(),false);
  for( intptr_t i = list.count() - 1; i >= 0 && !terminated_; i-- ){
    AsyncFile file(list[i]);
    try {
      file.open();
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      timeWait = true;
    }
    try {
      if( file.isOpen() ){
        AutoPtr<Message> message(newObject<Message>());
        file >> message;
        utf8::String suser, skey, host;
        splitString(message->value("#Recepient"),suser,skey,"@");
        bool deliverLocaly = false;
        Key2ServerLink * key2ServerLink = NULL;
        {
          Server::Data & data = server_.data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL ){
            if( key2ServerLink->server_.strcasecmp(myHost) == 0 ){
              deliverLocaly = true;
            }
            else {
              host = key2ServerLink->server_;
            }
          }
        }
        file.close();
        if( key2ServerLink == NULL ){
          uint64_t messageTTL = server_.config_->valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) +
            ".message_ttl",
            2678400u // 31 day
          );
          uint64_t messageTime = timeFromTimeString(message->value("#Relay.0.Received"));
          if( gettimeofday() - messageTime >= messageTTL * 1000000u ){
            utf8::String::Stream stream;
            stream << "Message " << message->id() << " TTL exhausted, removed.\n";
            message = NULL;
            remove(list[i]);
            stdErr.debug(1,stream);
          }
          else if( id_ >= 0 ){
            utf8::String::Stream stream;
            stream << "Message recepient " << message->value("#Recepient") <<
              " not found in database.\n"
            ;
            message = NULL;
            rename(file.fileName(),server_.spoolDir(-1) + getNameFromPathName(file.fileName()));
            stdErr.debug(1,stream);
          }
          else { // if collector
            timeWait = true;
          }
        }
        else if( deliverLocaly ){
// robot response
          bool process = true;
          if( message->isValue("#request.user.online") && message->value("#request.user.online").strlen() == 0 ){
            AutoLock<FiberInterlockedMutex> lock(server_.recvMailFibersMutex_);
            ServerFiber sfib(server_,suser,skey);
            ServerFiber * fib = server_.findRecvMailFiberNL(sfib);
            if( fib == NULL ){
              server_.sendRobotMessage(
                message->value("#Sender"),
                message->value("#Recepient"),
                message->value("#Sender.Sended"),
                "#request.user.online","no"
              );
              if( (bool) Mutant(message->isValue("#request.user.remove.message.if.offline")) ){
                utf8::String::Stream stream;
                stream << "Message " << message->id() <<
                  " received from " << message->value("#Sender") <<
                  " to " << message->value("#Recepient") <<
                  " removed, because '#request.user.online' == 'no' and '#request.user.remove.message.if.offline' == 'yes' \n"
                ;
                message = NULL;
                remove(list[i]);
                stdErr.debug(1,stream);
                process = false;
              }
            }
          }
////////////////
          if( process ){
            utf8::String userMailBox(server_.mailDir() + suser);
            utf8::String::Stream stream;
            stream << "Message " << message->id() <<
              " received from " << message->value("#Sender") <<
              " to " << message->value("#Recepient") <<
              " delivered localy to mailbox: " << userMailBox << "\n"
            ;
            utf8::String mailFile(includeTrailingPathDelimiter(userMailBox) + message->id() + ".msg");
            message = NULL;
            try {
              rename(list[i],mailFile);
            }
            catch( ... ){
              createDirectory(userMailBox);
              rename(list[i],mailFile);
            }
            stdErr.debug(0,stream);
          }
        }
        else {
          utf8::String::Stream stream;
          stream << "Message " << message->id() <<
            " received from " << message->value("#Sender") <<
            " to " << message->value("#Recepient") <<
            " is put in queue for delivery.\n"
          ;
          utf8::String mId(message->id());
          message = NULL;
          server_.sendMessage(host,mId,list[i]);
          stdErr.debug(0,stream);
        }
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      stdErr.debug(1,utf8::String::Stream() << "Invalid message " << list[i] << "\n");
      timeWait = true;
    }
    list.remove(i);
  }
}
//------------------------------------------------------------------------------
void SpoolWalker::fiberExecute()
{
  bool timeWait;
  while( !terminated_ ){
    try {
      processQueue(timeWait);
    }
    catch( ... ){
      timeWait = true;
    }
    if( terminated_ ) break;
    if( timeWait ){
      utf8::String iName(serverConfSectionName_[stStandalone]);
      uint64_t timeout = server_.config_->valueByPath(
        iName + (id_ >= 0 ? ".spool" : ".collector") + "_processing_interval",
        60u
      );
      sleep(timeout * 1000000u);
      stdErr.debug(9,utf8::String::Stream() << "Processing spool by timer... \n");
    }
    else {
      dcn_.monitor(excludeTrailingPathDelimiter(server_.spoolDir(id_)));
      stdErr.debug(9,utf8::String::Stream() << "Processing spool by monitor... \n");
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
MailQueueWalker::MailQueueWalker(Server & server,const utf8::String & host)
  : server_(server), host_(host)
{
}
//------------------------------------------------------------------------------
void MailQueueWalker::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  password = server_.config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  encryption = server_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = server_.config_->section("compression").text(utf8::String(),"default");
  compressionType = server_.config_->section("compression").value("type","default");
  crc = server_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = server_.config_->section("compression").value("level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
  bool noAuth = server_.config_->value("noauth",false);
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
void MailQueueWalker::connectHost(bool & online)
{
  if( !online ){
    close();
    ksock::SockAddr address;
    try {
      address.resolve(host_,defaultPort);
      try {
        connect(address);
        try {
          auth();
          try {
            *this << uint8_t(cmSelectServerType) << uint8_t(stStandalone);
            getCode();
            online = true;
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
          }
        }
        catch( ExceptionSP & e ){
          e->writeStdError();
          stdErr.debug(3,utf8::String::Stream() <<
            "Authentification to host " << host_ << " failed.\n"
          );
        }
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        stdErr.debug(3,utf8::String::Stream() <<
          "Unable to connect. Host " << host_ << " unreachable.\n"
        );
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      stdErr.debug(1,utf8::String::Stream() << "Unable to resolve " << host_ << "\n");
    }
  }
  uint64_t cec;
  {
    Server::Data & data = server_.data(stStandalone);
    AutoMutexWRLock<FiberMutex> lock(data.mutex_);
    ServerInfo * si = data.servers_.find(host_);
    if( si == NULL ) data.registerServerNL(host_);
    if( online ){
      si->connectErrorCount_ = 0;
      si->lastFailedConnectTime_ = 0;
      cec = 0;
    }
    else {
      si->connectErrorCount_++;
      si->lastFailedConnectTime_ = gettimeofday();
      cec = fibonacci(si->connectErrorCount_);
      stdErr.debug(7,utf8::String::Stream() <<
        "mqueue: Wait " << cec <<
        " seconds before connect to host " <<
        host_ << " because previous connect try failed.\n"
      );
      cec *= 1000000u;
      uint64_t mwt = (uint64_t) server_.config_->parse().valueByPath(
        utf8::String(serverConfSectionName_[stStandalone]) +
        ".max_wait_time_before_try_connect",
        600u
      ) * 1000000u;
      if( cec > mwt ) cec = mwt;
    }
  }
  if( !online ) sleep(cec);
}
//------------------------------------------------------------------------------
void MailQueueWalker::main()
{
  try {
    bool online = false;
    while( !terminated_ ){
      connectHost(online);
      if( online ){
        Message::Key mId;
        bool send;
        {
          AutoLock<FiberInterlockedMutex> lock(messagesMutex_);
          if( (send = messages_.count() > 0) ) mId = messages_.remove();
        }
        if( send ){
          uint64_t restFrom, remainder, rb = recvBytes(), sb = sendBytes();
          *this << uint8_t(cmSendMail) << mId << true >> restFrom;
          AsyncFile file;
          file.fileName(server_.mqueueDir() + mId + ".msg").readOnly(true).open().seek(restFrom);
          *this << (remainder = file.size() - restFrom);
          AutoPtr<uint8_t> b;
          size_t bl = getpagesize() * 16;
          b.alloc(bl);
          while( remainder > 0 ){
            uint64_t l = remainder > bl ? bl : remainder;
            file.read(b,l);
            write(b,l);
            remainder -= l;
          }
          getCode();
          stdErr.debug(0,utf8::String::Stream() <<
            "Message " << mId <<
            " sended to " << host_ << ", traffic " <<
            recvBytes() - rb + sendBytes() - sb << "\n"
          );
          file.close();
          remove(file.fileName());
        }
        else {
          uint64_t inactivityTime = (uint64_t) server_.config_->parse().valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) + ".mqueue_fiber_inactivity_time",
            60u
          ) * 1000000u;
          if( !semaphore_.timedWait(inactivityTime) ){
            *this << uint8_t(cmQuit);
            getCode();
            break;
          }
        }
      }
    }
  }
  catch( ExceptionSP & ){
    server_.removeSender(*this);
    throw;
  }
  server_.removeSender(*this);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
NodeClient::~NodeClient()
{
}
//------------------------------------------------------------------------------
NodeClient::NodeClient(Server & server) :
  server_(server), dataType_(stNone), periodicaly_(false)
{
}
//------------------------------------------------------------------------------
NodeClient * NodeClient::newClient(Server & server,ServerType dataType,const utf8::String & nodeHostName,bool periodicaly)
{
  NodeClient * p = newObjectV<NodeClient>(server);
  p->dataType_ = dataType;
  p->nodeHostName_ = nodeHostName;
  p->periodicaly_ = periodicaly;
  return p;
}
//------------------------------------------------------------------------------
void NodeClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void NodeClient::checkCode(int32_t code,int32_t noThrowCode1,int32_t noThrowCode2)
{
  if( code != eOK && code != noThrowCode1 && code != noThrowCode1)
    newObject<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
int32_t NodeClient::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode);
  return r;
}
//------------------------------------------------------------------------------
int32_t NodeClient::getCode(int32_t noThrowCode1,int32_t noThrowCode2)
{
  int32_t r;
  *this >> r;
  checkCode(r,noThrowCode1,noThrowCode2);
  return r;
}
//------------------------------------------------------------------------------
void NodeClient::auth()
{
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(server_.config_->parse().override().value("max_send_size",getpagesize()));
  user = server_.config_->text("user","system");
  password = server_.config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  encryption = server_.config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = server_.config_->section("encryption").value("threshold",1024 * 1024);
  compression = server_.config_->section("compression").text(utf8::String(),"default");
  compressionType = server_.config_->section("compression").value("type","default");
  crc = server_.config_->section("compression").value("crc","default");
  uintptr_t compressionLevel = server_.config_->section("compression").value("level",3);
  bool optimize = server_.config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = server_.config_->section("compression").value("buffer_size",getpagesize());
  bool noAuth = server_.config_->value("noauth",false);
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
void NodeClient::sweepHelper(ServerType serverType)
{
  utf8::String::Stream stream;
  stream << serverTypeName_[serverType] << ": sweep in " << serverTypeName_[serverType] << " database\n";
  uint64_t ttl = server_.config_->valueByPath(utf8::String(serverConfSectionName_[serverType]) + ".ttl","");
  uint64_t ttr = server_.config_->valueByPath(utf8::String(serverConfSectionName_[serverType]) + ".ttr","");
  if( server_.data(serverType).sweep(ttl * 1000000u,ttr * 1000000u,&stream) )
    stdErr.debug(5,stream);
}
//------------------------------------------------------------------------------
void NodeClient::main()
{
  if( periodicaly_ ) checkMachineBinding(
    server_.config_->parse().value("machine_key"),true
  );
  server_.data(stStandalone).registerServer(
    ServerInfo(server_.bindAddrs()[0].resolve(defaultPort),stStandalone)
  );
  intptr_t i;
  utf8::String server, host;
  Server::Data & data = server_.data(dataType_);
  try {
    bool tryConnect, connected, exchanged, doWork;
    do {
      stdErr.setDebugLevels(server_.config_->parse().value("debug_levels","+0,+1,+2,+3"));
      if( periodicaly_ ) checkMachineBinding(server_.config_->value("machine_key"),true);
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
          host = server_.config_->parse().valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) + ".node",""
          );
          if( server.strlen() > 0 && host.strlen() > 0 ) server += ",";
          server += host;
        }
        else {
          server = nodeHostName_;
        }
        i = enumStringParts(server);
        if( i <= 0 ) exchanged = true;
        while( !exchanged && !terminated_ && --i >= 0 ){
          try {
            ksock::SockAddr remoteAddress;
            remoteAddress.resolve(stringPartByNo(server,i),defaultPort);
            host = remoteAddress.resolve(defaultPort);
            close();
            tryConnect = false;
            uintptr_t cec = 0;
            uint64_t lastFailedConnectTime = 0;
            if( !periodicaly_ ){
              AutoMutexWRLock<FiberMutex> lock(data.mutex_);
              ServerInfo * si = data.servers_.find(host);
              if( si == NULL ){
                //assert( dataType_ == stNode );
                data.registerServerNL(ServerInfo(host,dataType_));
                si = data.servers_.find(host);
                assert( si != NULL );
              }
              cec = si->connectErrorCount_;
              lastFailedConnectTime = si->lastFailedConnectTime_;
            }
            if( cec > 0 ){
              uintptr_t mwt = server_.config_->parse().valueByPath(
                utf8::String(serverConfSectionName_[dataType_]) +
                ".max_wait_time_before_try_connect",
                600u
              );
              cec = (uintptr_t) fibonacci(cec);
              if( cec > mwt ) cec = mwt;
              cec *= 1000000u;
              //sleep(cec * 1000000u);
            }
            if( (uint64_t) gettimeofday() >= cec + lastFailedConnectTime ){
              tryConnect = true;
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
              *this << uint8_t(cmSelectProtocol) << uint8_t(1);
              bool useProto1 = getCode(eInvalidCommand,eInvalidProtocol) == eOK;
              utf8::String hostDB(host);
              if( useProto1 ) hostDB = hostDB + " " + serverTypeName_[stNode];
              data.registerServer(ServerInfo(host,stNode));
              Server::Data rdata, ldata, diff, dump;
              uint64_t rStartTime;
              *this << uint8_t(cmRegisterDB) <<
                uint32_t(ksock::api.ntohs(server_.bindAddrs()[0].addr4_.sin_port)) <<
                uint64_t(getProcessStartTime()) >>
                rStartTime;
              ;
              if( useProto1 ) *this << uint8_t(dataType_);
              bool fullDump = false;
              {
                AutoMutexRDLock<FiberMutex> lock(data.mutex_);
                ServerInfo * si = data.servers_.find(host);
                fullDump = si == NULL || si->stime_ != rStartTime;
                dump.orNL(data);
                ldata.orNL(data,fullDump ? utf8::String() : hostDB);
              }
              ldata.sendDatabaseNL(*this);
              rdata.recvDatabaseNL(*this);
              getCode();
              {
                AutoMutexWRLock<FiberMutex> lock(data.mutex_);
                rdata.setSendedToNL(hostDB);
                ldata.setSendedToNL(hostDB);
                data.orNL(rdata);
                data.orNL(ldata);
                ServerInfo * si = data.servers_.find(host);
                if( si != NULL ){
                  if( fullDump ) si->stime_ = rStartTime;
                  if( !periodicaly_ ){
                    si->connectErrorCount_ = 0;
                    si->lastFailedConnectTime_ = 0;
                  }
                }
              }
              exchanged = true;
              *this << uint8_t(cmQuit);
              getCode();
              stream.clear() << "NODE client: " << serverTypeName_[dataType_] <<
                " database " << (fullDump ? "full dump" : "changes") <<
                " sended to node " << host << "\n";
              ldata.dumpNL(stream);
              stdErr.debug(fullDump ? 7 : 6,stream);
              stream.clear() << "NODE client: " << serverTypeName_[dataType_] <<
                " database changes received from node " << host << "\n";
              rdata.dumpNL(stream);
              stdErr.debug(6,stream);
              diff.xorNL(dump,rdata);
              stream.clear() << "NODE client: changes stored in " <<
                serverTypeName_[dataType_] << " database.\n";
              diff.dumpNL(stream);
              stdErr.debug(5,stream);
            }
            catch( ExceptionSP & e ){
              e->writeStdError();
            }
            shutdown();
            close();
          }
          else if( !periodicaly_ ){
            AutoMutexWRLock<FiberMutex> lock(data.mutex_);
            ServerInfo * si = data.servers_.find(host);
            if( si != NULL && tryConnect ){
              si->connectErrorCount_++;
              si->lastFailedConnectTime_ = gettimeofday();
              stdErr.debug(7,utf8::String::Stream() <<
                "NODE client: Wait " << fibonacci(si->connectErrorCount_) << " seconds before connect to host " <<
                host << " because previous connect try failed.\n"
              );
            }
            sleep(uint64_t(1000000));
          }
        }
      }
      if( periodicaly_ ){
        sweepHelper(stStandalone);
        sweepHelper(stNode);
        uint64_t timeout = server_.config_->parse().
          valueByPath(
            utf8::String(serverConfSectionName_[stStandalone]) + ".exchange_interval",
            600u
          );
        sleep(timeout * 1000000u);
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
