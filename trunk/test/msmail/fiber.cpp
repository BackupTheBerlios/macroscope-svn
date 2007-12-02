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
  server_(&server), serverType_(stNone), protocol_(0), user_(user), key_(key),
  idsAutoDrop_(ids_)
{
}
//------------------------------------------------------------------------------
void ServerFiber::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  return server_->config_->section("users").isSection(user);
}
//------------------------------------------------------------------------------
utf8::String ServerFiber::getUserPassword(const utf8::String & user,const AuthParams * ap)
{
  if( server_->config_->section("users").isSection(user) ){
    if( ap != NULL ){
      ap->maxRecvSize_ = server_->config_->valueByPath("users." + user + ".max_recv_size",ap->maxRecvSize_);
      ap->maxSendSize_ = server_->config_->valueByPath("users." + user + ".max_send_size",ap->maxSendSize_);
      ap->recvTimeout_ = server_->config_->valueByPath("users." + user + ".recv_timeout",ap->recvTimeout_);
      if( ap->recvTimeout_ != ~uint64_t(0) ) ap->recvTimeout_ *= 1000000u;
      ap->sendTimeout_ = server_->config_->valueByPath("users." + user + ".send_timeout",ap->sendTimeout_);
      if( ap->sendTimeout_ != ~uint64_t(0) ) ap->sendTimeout_ *= 1000000u;
      ap->encryption_ = server_->config_->textByPath("users." + user + ".encryption.",ap->encryption_);
      ap->threshold_ = server_->config_->valueByPath("users." + user + ".encryption.threshold",ap->threshold_);
      ap->compression_ = server_->config_->textByPath("users." + user + ".compression.",ap->compression_);
      ap->compressionType_ = server_->config_->textByPath("users." + user + ".compression.type",ap->compressionType_);
      ap->crc_ = server_->config_->textByPath("users." + user + ".compression.crc",ap->crc_);
      ap->level_ = server_->config_->valueByPath("users." + user + ".compression.max_level",ap->level_);
      ap->optimize_ = server_->config_->valueByPath("users." + user + ".compression.optimize",ap->optimize_);
      ap->bufferSize_ = server_->config_->valueByPath("users." + user + ".compression.buffer_size",ap->bufferSize_);
    }
    return server_->config_->section("users").section(user).text("password");
  }
  return utf8::String();
}
//------------------------------------------------------------------------------
void ServerFiber::auth()
{
  AuthParams ap;
  ap.maxRecvSize_ = server_->config_->parse().override().value("max_recv_size",-1);
  ap.maxSendSize_ = server_->config_->value("max_send_size",-1);
  ap.recvTimeout_ = server_->config_->value("recv_timeout",-1);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = server_->config_->value("send_timeout",-1);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.encryption_ = server_->config_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = server_->config_->section("encryption").value("threshold",1024 * 1024);
  ap.compression_ = server_->config_->section("compression").text(utf8::String(),"default");
  ap.compressionType_ = server_->config_->section("compression").text("type","default");
  ap.crc_ = server_->config_->section("compression").text("crc","default");
  ap.level_ = server_->config_->section("compression").value("max_level",9);
  ap.optimize_ = server_->config_->section("compression").value("optimize",true);
  ap.bufferSize_ = server_->config_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.noAuth_ = false;
  if( server_->config_->isSection("users") ){
    ap.maxRecvSize_ = server_->config_->valueByPath("users.max_recv_size",ap.maxRecvSize_);
    ap.maxSendSize_ = server_->config_->valueByPath("users.max_send_size",ap.maxSendSize_);
    ap.recvTimeout_ = server_->config_->valueByPath("users.recv_timeout",ap.recvTimeout_);
    if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
    ap.sendTimeout_ = server_->config_->valueByPath("users.send_timeout",ap.sendTimeout_);
    if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
    ap.noAuth_ = server_->config_->valueByPath("users.noauth",ap.noAuth_);
    ap.encryption_ = server_->config_->textByPath("encryption.",ap.encryption_);
    ap.threshold_ = server_->config_->valueByPath("encryption.threshold",ap.threshold_);
    ap.compression_ = server_->config_->textByPath("compression.",ap.compression_);
    ap.compressionType_ = server_->config_->textByPath("compression.type",ap.compressionType_);
    ap.crc_ = server_->config_->textByPath("compression.crc",ap.crc_);
    ap.level_ = server_->config_->valueByPath("compression.max_level",ap.level_);
    ap.optimize_ = server_->config_->valueByPath("compression.optimize",ap.optimize_);
    ap.bufferSize_ = server_->config_->valueByPath("compression.buffer_size",ap.bufferSize_);
  }
  Error e = (Error) serverAuth(ap);
  if( e != eOK ){
    utf8::String::Stream stream;
    stream << "Authentification error from: " << remoteAddress().resolveAddr(~uintptr_t(0)) << "\n";
    stdErr.debug(9,stream);
    newObjectV1C2<Exception>(e,__PRETTY_FUNCTION__)->throwSP();
  }
}
//------------------------------------------------------------------------------
void ServerFiber::main()
{
  server_->config_->parse();
  stdErr.setDebugLevels(server_->config_->value("debug_levels","+0,+1,+2,+3"));
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
  if( !(bool) server_->config_->parse().valueByPath(
        utf8::String(serverConfSectionName[serverType_]) + ".enabled",true) ){
    utf8::String host(remoteAddress().resolveAddr(~uintptr_t(0)));
    utf8::String::Stream stream;
    stream << serverTypeName[serverType_] <<
      ": functional disabled, but client " << host << " attempt to register.\n";
    stdErr.debug(6,stream);
    terminate();
    return;
  }
  utf8::String host(remoteAddress().resolveAddr(~uintptr_t(0)));
  ServerInfo server(server_->bindAddrs()[0].resolveAddr(defaultPort),stStandalone);
  Server::Data & data = server_->data(serverType_);
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
  if( startNodeClient ) server_->startNodeClient(stStandalone);
  putCode(serverType_ == stStandalone ? eOK : eInvalidServerType);
  flush();
  if( serverType_ == stStandalone ){
    utf8::String::Stream stream;
    stream << serverTypeName[serverType_] << ": changes stored from client " << host << "\n";
    diff.dumpNL(stream);
    stdErr.debug(5,stream);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerDB()
{
  utf8::String::Stream stream;
  utf8::String host(remoteAddress().resolveAddr(~uintptr_t(0))), server, service;
  if( serverType_ != stNode ){
    terminate();
    stream << serverTypeName[serverType_] <<
      ": " << host << " ask for " << serverTypeName[serverType_] << " database\n";
    stdErr.debug(6,stream);
    return;
  }
  if( !(bool) server_->config_->parse().valueByPath(
        utf8::String(serverConfSectionName[serverType_]) + ".enabled",false) ){
    terminate();
    stream << serverTypeName[serverType_] <<
      ": functional disabled, but " << host << " attempt to register own database.\n";
    stdErr.debug(6,stream);
    return;
  }
  bool irsl = server_->config_->valueByPath(
    utf8::String(serverConfSectionName[serverType_]) + ".ignore_remote_send_list",false
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
      stream << serverTypeName[serverType_] <<
        ": remote host " << host << " send invalid own database type.\n";
      stdErr.debug(6,stream);
      return;
    }
  }
  splitString(host,server,service,":");
  host = server;
  if( port != defaultPort ) host += ":" + utf8::int2Str(port);
  utf8::String hostDB(host);
  if( protocol_ > 0 ) hostDB = hostDB + " " + serverTypeName[rdbt];
  Server::Data rdata, ldata, diff;
  rdata.recvDatabaseNL(*this);
  stream << serverTypeName[serverType_] <<
    ": database changes received from " << hostDB << "\n";
  rdata.dumpNL(stream);
  stdErr.debug(6,stream);
  Server::Data & data = server_->data(serverType_);
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
  if( dbChanged ) server_->startNodesExchange();
  stream.clear() << serverTypeName[serverType_] <<
    ": database " << (fullDump ? "full dump" : "changes") << " sended to " << hostDB << "\n";
  ldata.dumpNL(stream);
  stdErr.debug(fullDump ? 7 : 6,stream);
  stream.clear() << serverTypeName[serverType_] << ": changes stored\n";
  diff.dumpNL(stream);
  stdErr.debug(5,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::getDB()
{
  if( serverType_ != stStandalone && serverType_ != stNode ) return;
  utf8::String host(remoteAddress().resolveAddr(defaultPort));
  Server::Data tdata;
  tdata.ore(server_->data(serverType_));
  tdata.sendDatabaseNL(*this);
  putCode(eOK);
  flush();
  utf8::String::Stream stream;
  stream << serverTypeName[serverType_] <<
    ": database sended to client " << host << "\n";
  tdata.dumpNL(stream);
  stdErr.debug(8,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::sendMail() // client sending mail
{
  uintptr_t i;
  AutoPtr<Message> msg;
  utf8::String id;
  uint64_t rb = recvBytes(), sb = sendBytes(), remainder;
  bool rest;

  *this >> id >> rest;
  AsyncFile file;
  file.fileName(server_->incompleteDir() + id + ".msg").createIfNotExist(true).exclusive(true);
  if( rest ){
    file.open();
    *this << file.size() >> remainder;
    file.seek(file.size());
    AutoPtr<uint8_t> b;
    size_t bl = getpagesize() * 16;
    b.alloc(bl);
    while( remainder > 0 ){
      uint64_t l = remainder > bl ? bl : remainder;
      read(b,l);
      file.write(b,l);
      remainder -= l;
    }
    file.seek(0);
    msg = newObject<msmail::Message>();
    file >> msg;
    file.close();
    msg->file().exclusive(true);
  }
  else {
    msg = newObject<Message>();
    msg->file().fileName(file.fileName()).createIfNotExist(true).exclusive(true);
    *this >> msg;
  }
  AutoFileRemove afr(file.fileName());
  AutoPtr<Message> message;
  message.xchg(msg);
  utf8::String::Stream stream;
  stream << "Message " << message->id() <<
    " received from " << message->value("#Sender") <<
    " to " << message->value("#Recepient") <<
    ", traffic " <<
    recvBytes() - rb + sendBytes() - sb << "\n"
  ;
  utf8::String recepient, mid;
  if( !message->isValue("#Recepient",&recepient) || 
      recepient.trim().isNull() ||
      !message->isValue(messageIdKey,&mid) ||
      id.strcmp(mid) != 0 ){
    putCode(eInvalidMessage);
    return;
  }
  utf8::String relay;
  for( i = 0; i < ~uintptr_t(0); i++ ){
    relay = "#Relay." + utf8::int2Str(i);
    if( !message->isValue(relay) ) break;
  }
  message->value(relay,server_->bindAddrs()[0].resolveAddr(defaultPort));
  relay = relay + ".";
  message->value(relay + "Received",getTimeString(gettimeofday()));
  message->value(relay + "Process.Id",utf8::int2Str(ksys::getpid()));
  message->value(relay + "Process.StartTime",getTimeString(getProcessStartTime()));
  AsyncFile file2(file.fileName() + ".tmp");
  file2.createIfNotExist(true).removeAfterClose(true).exclusive(true).open();
  try {
    file2 << message;
    file2.removeAfterClose(false).close();
    rename(
      file2.fileName(),
      server_->spoolDir(id.hash(true) & (server_->spoolFibers_ - 1)) + id + ".msg"
    );
  }
  catch( ExceptionSP & e ){
    remove(file2.fileName());
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->code() == ERROR_ALREADY_EXISTS + errorOffset ){
#else
    if( e->code() == EEXIST ){
#endif
      stream << ", is duplicate, removed.\n";
      stdErr.debug(0,stream);
    }
    throw;
  }
  putCode(eOK);
  stdErr.debug(0,stream);
}
//------------------------------------------------------------------------------
void ServerFiber::processMailbox(
  const utf8::String & userMailBox,
  bool waitForMail,
  bool onlyNewMail,
  bool & wait)
{
  wait = true;
  utf8::String myHost(server_->bindAddrs()[0].resolveAddr(defaultPort));
  Vector<utf8::String> list;
  getDirList(list,userMailBox + "*.msg",utf8::String(),false);
  for( intptr_t i = list.count() - 1; i >= 0 && !terminated_; i-- ){
    utf8::String id(changeFileExt(getNameFromPathName(list[i]),""));
    bool isNewMessage = ids_.find(id) == NULL;
    AsyncFile file(list[i]);
    try {
      file.open();
      if( file.size() == 0 ) file.removeAfterClose(true).close();
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
    if( file.isOpen() && (isNewMessage || !onlyNewMail) ){
      AutoPtr<Message> message(newObject<Message>());
      try {
        file >> message;
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        file.removeAfterClose(true);
      }
      file.close();
      if( !file.removeAfterClose() ){
        assert( message->id().strcmp(id) == 0 );
        utf8::String suser, skey;
        splitString(message->value("#Recepient"),suser,skey,"@");
        bool lostSheep = true;
        {
          Server::Data & data = server_->data(stStandalone);
          AutoMutexRDLock<FiberMutex> lock(data.mutex_);
          Key2ServerLink * key2ServerLink = data.key2ServerLinks_.find(skey);
          if( key2ServerLink != NULL )
            lostSheep = key2ServerLink->server_.strcasecmp(myHost) != 0;
        }
        lostSheep = user_.strcasecmp(suser) != 0 || lostSheep;
        if( lostSheep ){
          try {
            rename(file.fileName(),server_->spoolDir(id.hash(true) & (server_->spoolFibers_ - 1)) + getNameFromPathName(list[i]));
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
          }
        }
        else {
          bool messageAccepted = true;
          if( skey.strcasecmp(key_) == 0 ){
            if( !waitForMail ) *this << bool(true);
            *this << *message.ptr() >> messageAccepted;
            putCode(i > 0 ? eOK : eLastMessage);
          }
          if( messageAccepted ){
            ids_.insert(*newObjectC1<Message::Key>(id),false);
          }
          if( wait && !messageAccepted ) wait = false;
        }
      }
    }
    list.remove(i);
  }
  flush();
}
//------------------------------------------------------------------------------
void ServerFiber::recvMail() // client receiving mail
{
  bool waitForMail, onlyNewMail, wait;
  *this >> user_ >> key_ >> waitForMail >> onlyNewMail;
  utf8::String userMailBox(includeTrailingPathDelimiter(server_->mailDir() + user_));
  putCode(eOK);
  server_->addRecvMailFiber(*this);
  try {
    server_->sendUserWatchdog(user_);
    while( !terminated_ ){
      processMailbox(userMailBox,waitForMail,onlyNewMail,wait);
      if( !waitForMail ){
        *this << bool(false);
        flush();
        break;
      }
      if( wait ){
        if( server_->findRecvMailFiber(*this) != this ) break;
        dcn_.monitor(excludeTrailingPathDelimiter(userMailBox));
        stdErr.debug(9,utf8::String::Stream() <<
          "Processing mailbox " <<
          excludeTrailingPathDelimiter(userMailBox) <<
          " by monitor... \n"
        );
      }
    }
  }
  catch( ExceptionSP & ){
    server_->remRecvMailFiber(*this);
    throw;
  }
  server_->remRecvMailFiber(*this);
}
//------------------------------------------------------------------------------
void ServerFiber::removeMail() // client remove mail
{
  int32_t e = eOK;
  utf8::String mailForUser, id;
  *this >> mailForUser >> id;
  utf8::String userMailBox(includeTrailingPathDelimiter(server_->mailDir() + mailForUser));
  utf8::String name(includeTrailingPathDelimiter(userMailBox) + id + ".msg");
  for( bool cont = true; cont;){
    try {
      remove(name);
      cont = false;
    }
    catch( ExceptionSP & ex ){
      switch( ex->code() ){
#if defined(__WIN32__) || defined(__WIN64__)
        case ERROR_FILE_NOT_FOUND + errorOffset :
#else
        case ENOENT :
#endif
          e = eInvalidMessageId;
          cont = false;
          break;
#if defined(__WIN32__) || defined(__WIN64__)
        case ERROR_SHARING_VIOLATION + errorOffset :
        case ERROR_LOCK_VIOLATION + errorOffset :
#else
        case EACCES :
#endif
          break;
        default :
          ex->writeStdError();
          e = eFail;
          cont = false;
      }
    }
    if( cont ) ksleep(100000);
  }
  putCode(e);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SpoolWalker::~SpoolWalker()
{
}
//------------------------------------------------------------------------------
SpoolWalker::SpoolWalker(Server & server,intptr_t id) : server_(&server), id_(id)
{
}
//------------------------------------------------------------------------------
void SpoolWalker::processQueue(bool & timeWait)
{
  timeWait = false;
  utf8::String myHost(server_->bindAddrs()[0].resolveAddr(defaultPort));
  Vector<utf8::String> list;
  getDirList(list,server_->spoolDir(id_) + "*.msg",utf8::String(),false);
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
          Server::Data & data = server_->data(stStandalone);
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
          uint64_t messageTTL = server_->config_->valueByPath(
            utf8::String(serverConfSectionName[stStandalone]) +
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
// robot response
            if( server_->processRequestServerOnline(message,list[i]) ){
/////////////////
              utf8::String::Stream stream;
              stream << "Message recepient " << message->value("#Recepient") <<
                " not found in database.\n"
              ;
              message = NULL;
              rename(file.fileName(),server_->spoolDir(-1) + getNameFromPathName(file.fileName()),true,true);
              stdErr.debug(1,stream);
            }
          }
          else { // if collector          
            timeWait = true;
          }
        }
        else if( deliverLocaly ){
// robot response
          if( server_->processRequestUserOnline(message,list[i],suser,skey) ){
////////////////
            utf8::String userMailBox(includeTrailingPathDelimiter(server_->mailDir() + suser));
            utf8::String::Stream stream;
            stream << "Message " << message->id() <<
              " received from " << message->value("#Sender") <<
              " to " << message->value("#Recepient")
            ;
            utf8::String mailFile(userMailBox + message->id() + ".msg");
            message = NULL;
            try {
              rename(list[i],mailFile);
              stream << " delivered localy to mailbox: " << userMailBox << "\n";
            }
            catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
              if( e->code() != ERROR_ALREADY_EXISTS + errorOffset ) throw;
#else
              if( e->code() != EEXIST ) throw;
#endif
              remove(list[i]);
              e->writeStdError();
              stream << " is duplicate, removed.\n";
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
          server_->sendMessage(host,mId,list[i]);
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
  if( id_ < 0 ){ // if collector
    utf8::String::Stream stream;
    utf8::String name(server_->spoolDir(id_) + "clear_node_database.ctrl");
    if( stat(name) ){
      server_->data(stNode).clear();
      remove(name,true);
      stream << serverTypeName[stNode] << " database droped by user request.\n";
      stdErr.debug(30,stream);
    }
    name = server_->spoolDir(id_) + "clear_standalone_database.ctrl";
    if( stat(name) ){
      server_->data(stStandalone).clear();
      remove(name,true);
      stream << serverTypeName[stStandalone] << " database droped by user request.\n";
      stdErr.debug(30,stream);
    }
    name = server_->spoolDir(id_) + "clear_database.ctrl";
    if( stat(name) ){
      server_->data(stNode).clear();
      server_->data(stStandalone).clear();
      remove(name,true);
      stream << serverTypeName[stNode] << " and " <<
        serverTypeName[stStandalone] << " database droped by user request.\n";
      stdErr.debug(30,stream);
    }
  }
}
//------------------------------------------------------------------------------
void SpoolWalker::fiberExecute()
{
  utf8::String iName(serverConfSectionName[stStandalone]);
  utf8::String vName(iName + (id_ >= 0 ? ".spool" : ".collector") + "_processing_interval");
  bool timeWait;
  while( !terminated_ ){
    try {
      processQueue(timeWait);
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      timeWait = true;
    }
    if( terminated_ ) break;
    if( timeWait ){
      uint64_t timeout = server_->config_->valueByPath(vName,60u);
      ksleep(timeout * 1000000u);
      stdErr.debug(9,utf8::String::Stream() << "Processing spool by timer... \n");
    }
    else {
      dcn_.monitor(excludeTrailingPathDelimiter(server_->spoolDir(id_)));
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
MailQueueWalker::MailQueueWalker(Server & server) :
  server_(&server), messagesAutoDrop_(messages_)
{
}
//------------------------------------------------------------------------------
MailQueueWalker::MailQueueWalker(Server & server,const utf8::String & host)
  : server_(&server), host_(host), messagesAutoDrop_(messages_)
{
}
//------------------------------------------------------------------------------
void MailQueueWalker::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  AuthParams ap;
  ap.maxSendSize_ = server_->config_->value("max_send_size",-1);
  ap.maxRecvSize_ = server_->config_->value("max_recv_size",-1);
  ap.recvTimeout_ = server_->config_->value("recv_timeout",-1);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = server_->config_->value("send_timeout",-1);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.user_ = server_->config_->text("user","system");
  ap.password_ = server_->config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  ap.encryption_ = server_->config_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = server_->config_->section("encryption").value("threshold",1024 * 1024);
  ap.compression_ = server_->config_->section("compression").text(utf8::String(),"default");
  ap.compressionType_ = server_->config_->section("compression").value("type","default");
  ap.crc_ = server_->config_->section("compression").value("crc","default");
  ap.level_ = server_->config_->section("compression").value("level",9);
  ap.optimize_ = server_->config_->section("compression").value("optimize",true);
  ap.bufferSize_ = server_->config_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.noAuth_ = server_->config_->value("noauth",false);
  checkCode(clientAuth(ap));
}
//------------------------------------------------------------------------------
void MailQueueWalker::connectHost(bool & online,bool & mwt)
{
  mwt = false;
  if( !online ){
    close();
    ksock::SockAddr address;
    try {
      address.resolveName(host_,defaultPort);
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
    Server::Data & data = server_->data(stStandalone);
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
      uint64_t mwtv = (uint64_t) server_->config_->parse().valueByPath(
        utf8::String(serverConfSectionName[stStandalone]) +
        ".max_wait_time_before_try_connect",
        600u
      ) * 1000000u;
      mwt = cec > mwtv;
      if( mwt ) cec = mwtv;
    }
  }
  if( !online ) ksleep(cec);
}
//------------------------------------------------------------------------------
void MailQueueWalker::main()
{
  try {
    server_->config_->parse();
    bool online = false, mwt;
    while( !terminated_ ){
      connectHost(online,mwt);
      if( online ){
        Message::Key * mId = NULL;
        {
          AutoLock<FiberInterlockedMutex> lock(messagesMutex_);
          if( messages_.count() > 0 ) mId = &Message::Key::listNodeObject(*messages_.first());
        }
        if( mId != NULL ){
          uint64_t restFrom, remainder, rb = recvBytes(), sb = sendBytes();
          *this << uint8_t(cmSendMail) << *mId << true >> restFrom;
          AsyncFile file;
          file.fileName(server_->mqueueDir() + *mId + ".msg").readOnly(true).open().seek(restFrom);
          *this << (remainder = file.size() - restFrom);
          AutoPtr<uint8_t> b;
          size_t bl = getpagesize() * 16;
          b.alloc(bl);
          while( remainder > 0 ){
            uint64_t l = remainder > bl ? bl : remainder;
            file.readBuffer(b,l);
            write(b,l);
            remainder -= l;
          }
          getCode();
          stdErr.debug(0,utf8::String::Stream() <<
            "Message " << *mId <<
            " sended to " << host_ << ", traffic " <<
            recvBytes() - rb + sendBytes() - sb << "\n"
          );
          file.close();
          remove(file.fileName());
          AutoLock<FiberInterlockedMutex> lock(messagesMutex_);
          messages_.drop(*mId);
        }
        else {
          uint64_t inactivityTime = (uint64_t) server_->config_->valueByPath(
            utf8::String(serverConfSectionName[stStandalone]) + ".mqueue_fiber_inactivity_time",
            60u
          ) * 1000000u;
          if( !semaphore_.timedWait(inactivityTime) ){
            *this << uint8_t(cmQuit);
            getCode();
            break;
          }
        }
      }
      else {        
// robot response
        AutoLock<FiberInterlockedMutex> lock(messagesMutex_);
        for( EmbeddedListNode<Message::Key> * node = messages_.first(); node != NULL; node = node->next() ){
          Message::Key * mId = &Message::Key::listNodeObject(*node);
          AsyncFile file(server_->mqueueDir() + *mId + ".msg");
          AutoPtr<Message> message(newObject<Message>());
          try {
            file.open();
            file >> message;
            file.close();
            server_->processRequestServerOnline(message,file.fileName());
          }
          catch( ExceptionSP & e ){
            e->writeStdError();
          }
        }
/////////////////
        if( mwt ) break;
      }
    }
  }
  catch( ExceptionSP & ){
    server_->removeSender(*this);
    throw;
  }
  server_->removeSender(*this);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
NodeClient::~NodeClient()
{
}
//------------------------------------------------------------------------------
NodeClient::NodeClient(Server & server) :
  server_(&server), dataType_(stNone), periodicaly_(false)
{
}
//------------------------------------------------------------------------------
NodeClient * NodeClient::newClient(Server & server,ServerType dataType,const utf8::String & nodeHostName,bool periodicaly)
{
  NodeClient * p = newObjectR1<NodeClient>(server);
  p->dataType_ = dataType;
  p->nodeHostName_ = nodeHostName;
  p->periodicaly_ = periodicaly;
  return p;
}
//------------------------------------------------------------------------------
void NodeClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void NodeClient::checkCode(int32_t code,int32_t noThrowCode1,int32_t noThrowCode2)
{
  if( code != eOK && code != noThrowCode1 && code != noThrowCode1)
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  AuthParams ap;
  ap.maxSendSize_ = server_->config_->value("max_send_size",-1);
  ap.maxRecvSize_ = server_->config_->value("max_recv_size",-1);
  ap.recvTimeout_ = server_->config_->value("recv_timeout",-1);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = server_->config_->value("send_timeout",-1);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.user_ = server_->config_->text("user","system");
  ap.password_ = server_->config_->text("password","sha256:jKHSsCN1gvGyn07F4xp8nvoUtDIkANkxjcVQ73matyM");
  ap.encryption_ = server_->config_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = server_->config_->section("encryption").value("threshold",1024 * 1024);
  ap.compression_ = server_->config_->section("compression").text(utf8::String(),"default");
  ap.compressionType_ = server_->config_->section("compression").value("type","default");
  ap.crc_ = server_->config_->section("compression").value("crc","default");
  ap.level_ = server_->config_->section("compression").value("level",9);
  ap.optimize_ = server_->config_->section("compression").value("optimize",true);
  ap.bufferSize_ = server_->config_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.noAuth_ = server_->config_->value("noauth",false);
  checkCode(clientAuth(ap));
}
//------------------------------------------------------------------------------
void NodeClient::sweepHelper(ServerType serverType)
{
  utf8::String::Stream stream;
  stream << serverTypeName[serverType] << ": sweep in " << serverTypeName[serverType] << " database\n";
  uint64_t ttl = server_->config_->valueByPath(utf8::String(serverConfSectionName[serverType]) + ".ttl","");
  uint64_t ttr = server_->config_->valueByPath(utf8::String(serverConfSectionName[serverType]) + ".ttr","");
  if( server_->data(serverType).sweep(ttl * 1000000u,ttr * 1000000u,&stream) )
    stdErr.debug(5,stream);
}
//------------------------------------------------------------------------------
void NodeClient::main()
{
  server_->data(stStandalone).registerServer(
    ServerInfo(server_->bindAddrs()[0].resolveAddr(defaultPort),stStandalone)
  );
  intptr_t i;
  utf8::String server, host, enabledPath(utf8::String(serverConfSectionName[stStandalone]) + ".enabled");
  Server::Data & data = server_->data(dataType_);
  try {
    bool tryConnect = false, connected, exchanged, doWork;
    do {
      server_->config_->parse();
      stdErr.setDebugLevels(server_->config_->value("debug_levels","+0,+1,+2,+3"));
#ifndef DOXYGEN_SHOULD_SKIP_THIS
      if( periodicaly_ ) checkMachineBinding(server_->config_->value("machine_key"),true);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
      connected = exchanged = false;
      {
        AutoLock<FiberInterlockedMutex> lock(server_->nodeClientMutex_);
        doWork = 
          dataType_ != stStandalone ||
          (server_->nodeClient_ == NULL && periodicaly_) ||
          server_->nodeClient_ == this
        ;
      }
      if( doWork && periodicaly_ && !(bool) server_->config_->valueByPath(enabledPath,true) ) doWork = false;
      if( doWork ){
        if( dataType_ == stStandalone ){
          server = server_->data(stStandalone).getNodeList();
          host = server_->data(stNode).getNodeList();
          if( !server.isNull() && !host.isNull() ) server += ",";
          server += host;
          host = server_->config_->parse().valueByPath(
            utf8::String(serverConfSectionName[stStandalone]) + ".node",""
          );
          if( !server.isNull() && !host.isNull() ) server += ",";
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
            remoteAddress.resolveName(stringPartByNo(server,i),defaultPort);
            host = remoteAddress.resolveAddr(defaultPort);
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
              uintptr_t mwt = server_->config_->parse().valueByPath(
                utf8::String(serverConfSectionName[dataType_]) +
                ".max_wait_time_before_try_connect",
                600u
              );
              cec = (uintptr_t) fibonacci(cec);
              if( cec > mwt ) cec = mwt;
              cec *= 1000000u;
              //ksleep(cec * 1000000u);
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
              if( useProto1 ) hostDB = hostDB + " " + serverTypeName[stNode];
              data.registerServer(ServerInfo(host,stNode));
              Server::Data rdata, ldata, diff, dump;
              uint64_t rStartTime;
              *this << uint8_t(cmRegisterDB) <<
                uint32_t(ksock::api.ntohs(server_->bindAddrs()[0].addr4_.sin_port)) <<
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
              stream.clear() << "NODE client: " << serverTypeName[dataType_] <<
                " database " << (fullDump ? "full dump" : "changes") <<
                " sended to node " << host << "\n";
              ldata.dumpNL(stream);
              stdErr.debug(fullDump ? 7 : 6,stream);
              stream.clear() << "NODE client: " << serverTypeName[dataType_] <<
                " database changes received from node " << host << "\n";
              rdata.dumpNL(stream);
              stdErr.debug(6,stream);
              diff.xorNL(dump,rdata);
              stream.clear() << "NODE client: changes stored in " <<
                serverTypeName[dataType_] << " database.\n";
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
            {
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
            }
            ksleep(uint64_t(1000000));
          }
        }
      }
      if( periodicaly_ ){
        sweepHelper(stStandalone);
        sweepHelper(stNode);
        uint64_t timeout = server_->config_->
          valueByPath(
            utf8::String(serverConfSectionName[stStandalone]) + ".exchange_interval",
            600u
          );
        ksleep(timeout * 1000000u);
      }
    } while( periodicaly_ && !terminated_ );
    AutoLock<FiberInterlockedMutex> lock(server_->nodeClientMutex_);
    if( !periodicaly_ && !exchanged ){
      if( dataType_ == stStandalone ) server_->skippedNodeClientStarts_++;
      if( dataType_ == stNode ) server_->skippedNodeExchangeStarts_++;
    }
  }
  catch( ExceptionSP & ){
    if( !periodicaly_ ) server_->clearNodeClient(this);
    throw;
  }
  if( !periodicaly_ ) server_->clearNodeClient(this);
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
