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
      case cmRegisterUser :
        registerUser();
        break;
      case cmRegisterKey :
        registerKey();
        break;
      case cmRegisterGroup :
        registerGroup();
        break;
      case cmRegisterServer :
        registerServer();
        break;
      case cmRegisterUser2KeyLink :
        registerUser2KeyLink();
        break;
      case cmRegisterKey2GroupLink :
        registerKey2GroupLink();
        break;
      case cmGetUserList :
        getUserList();
        break;
      case cmGetKeyList :
        getKeyList();
        break;
      case cmGetGroupList :
        getGroupList();
        break;
      case cmGetServerList :
        getServerList();
        break;
      case cmSendMail :
        break;
      case cmRecvMail :
        break;
      default : // unrecognized or unsupported command, terminate
        putCode(eInvalidCommand);
        terminate();
    }
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerUser()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<UserInfo> user(new UserInfo);
  *this >> user;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.users_.find(user) == NULL ){
    data.userList_.safeAdd(user.ptr(NULL));
    data.users_.insert(user);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerKey()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<KeyInfo> key(new KeyInfo);
  *this >> key;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.keys_.find(key) == NULL ){
    data.keyList_.safeAdd(key.ptr(NULL));
    data.keys_.insert(key);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerGroup()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<GroupInfo> group(new GroupInfo);
  *this >> group;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.groups_.find(group) == NULL ){
    data.groupList_.safeAdd(group.ptr(NULL));
    data.groups_.insert(group);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerServer()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<ServerInfo> server(new ServerInfo);
  *this >> server;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.servers_.find(server) == NULL ){
    data.serverList_.safeAdd(server.ptr(NULL));
    data.servers_.insert(server);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::registerUser2KeyLink()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<UserInfo> user(new UserInfo);
  AutoPtr<KeyInfo> key(new KeyInfo);
  *this >> user >> key;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.users_.find(user) == NULL ){
    data.userList_.safeAdd(user.ptr(NULL));
    data.users_.insert(user);
  }
  if( data.keys_.find(key) == NULL ){
    data.keyList_.safeAdd(key.ptr(NULL));
    data.keys_.insert(key);
  }
  intptr_t c, i = key->users_.bSearchCase(user->name_,c);
  if( c != 0 ) key->users_.insert(i + (i > 0),user->name_);
}
//------------------------------------------------------------------------------
void ServerFiber::registerKey2GroupLink()
{
  Server::Data & data = server_.data(serverType_);
  AutoPtr<KeyInfo> key(new KeyInfo);
  AutoPtr<GroupInfo> group(new GroupInfo);
  *this >> key >> group;
  AutoLock<FiberInterlockedMutex> lock(data.mutex_);
  if( data.keys_.find(key) == NULL ){
    data.keyList_.safeAdd(key.ptr(NULL));
    data.keys_.insert(key);
  }
  if( data.groups_.find(group) == NULL ){
    data.groupList_.safeAdd(group.ptr(NULL));
    data.groups_.insert(group);
  }
  intptr_t c, i = group->keys_.bSearchCase(key->key_,c);
  if( c != 0 ) group->keys_.insert(i + (i > 0),key->key_);
}
//------------------------------------------------------------------------------
void ServerFiber::getUserList()
{
  int64_t mtime;
  *this >> mtime;
  Server::Data & data = server_.data(serverType_);
  data.addRef(data.userList_);
  try {
    for( intptr_t i = data.userList_.count() - 1; i >= 0; i-- )
      if( data.userList_[i].mtime_ > mtime ) *this << data.userList_[i].name_;
    *this << utf8::String(); // end of stream
  }
  catch( ... ){
    data.remRef(data.userList_,data.users_);
    throw;
  }
  data.remRef(data.userList_,data.users_);
}
//------------------------------------------------------------------------------
void ServerFiber::getKeyList()
{
  int64_t mtime;
  *this >> mtime;
  Server::Data & data = server_.data(serverType_);
  data.addRef(data.keyList_);
  try {
    for( intptr_t i = data.keyList_.count() - 1; i >= 0; i-- )
      if( data.keyList_[i].mtime_ > mtime ) *this << data.keyList_[i].key_;
    *this << utf8::String(); // end of stream
  }
  catch( ... ){
    data.remRef(data.keyList_,data.keys_);
    throw;
  }
  data.remRef(data.keyList_,data.keys_);
}
//------------------------------------------------------------------------------
void ServerFiber::getGroupList()
{
  int64_t mtime;
  *this >> mtime;
  Server::Data & data = server_.data(serverType_);
  data.addRef(data.groupList_);
  try {
    for( intptr_t i = data.groupList_.count() - 1; i >= 0; i-- )
      if( data.groupList_[i].mtime_ > mtime ) *this << data.groupList_[i].name_;
    *this << utf8::String(); // end of stream
  }
  catch( ... ){
    data.remRef(data.groupList_,data.groups_);
    throw;
  }
  data.remRef(data.groupList_,data.groups_);
}
//------------------------------------------------------------------------------
void ServerFiber::getServerList()
{
  int64_t mtime;
  *this >> mtime;
  Server::Data & data = server_.data(serverType_);
  data.addRef(data.serverList_);
  try {
    for( intptr_t i = data.serverList_.count() - 1; i >= 0; i-- )
      if( data.serverList_[i].mtime_ > mtime ) *this << data.serverList_[i].name_;
    *this << utf8::String(); // end of stream
  }
  catch( ... ){
    data.remRef(data.serverList_,data.servers_);
    throw;
  }
  data.remRef(data.serverList_,data.servers_);
}
//------------------------------------------------------------------------------
void ServerFiber::sendMail()
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
    relay += ".";
    message->value(relay + "#Received",getTimeString(gettimeofday()));
    message->value(relay + "#Process.Id",utf8::int2Str(ksys::getpid()));
    message->value(relay + "#Process.StartTime",getTimeString(getProcessStartTime()));
    utf8::String spool(server_.spoolDir());
    AsyncFile file(spool + message->value(messageIdKey));
    file.removeAfterClose(true);
    file.exclusive(true);
    file.open();
    file << message;
    file.removeAfterClose(false);
    file.close();
    putCode(eOK);
  }
}
//------------------------------------------------------------------------------
void ServerFiber::recvMail()
{
  utf8::String mailForUser, mailForKey;
  uint8_t waitForMail;
  *this >> mailForUser >> mailForKey >> waitForMail;
  utf8::String userMailBox(server_.mailDir() + mailForUser);
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
void SpoolWalker::fiberExecute()
{
  intptr_t i;
  DirectoryChangeNotification dcn;
  utf8::String spool(server_.spoolDir());
  while( !terminated_ ){
    bool wait = true;
    Vector<utf8::String> list;
    getDirListAsync(list,spool + "*.msg",utf8::String(),false);
    while( list.count() > 0 ){
      i = server_.rnd_->random(list.count());
      AsyncFile ctrl(list[i] + ".lck");
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
        wait = e->code() != ERROR_SHARING_VIOLATION;
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
          Server::Data & data = server_.data(stStandalone);
          bool deliverLocaly;
          {
            AutoLock<FiberInterlockedMutex> lock(data.mutex_);
            KeyInfo * key = data.keys_.find(skey);
            deliverLocaly = key != NULL && key->server_.strcasecmp(ksock::SockAddr::gethostname()) == 0;
          }
          file.close();
          if( deliverLocaly ){
            utf8::String userMailBox(server_.mailDir() + suser);
            createDirectoryAsync(userMailBox);
            renameAsync(list[i],includeTrailingPathDelimiter(userMailBox) + message.id());
          }
          else {
            renameAsync(list[i],server_.mqueueDir() + message.id());
          }
        }
      }
      catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_INVALID_DATA + errorOffset ) throw;
#else
        if( e->code() != EINVAL ) throw;
#endif
        stdErr.log(lmWARNING,utf8::String::Stream() << "Invalid message " << list[i] << "\n");
      }
      list.remove(i);
    }
    if( wait ){
      dcn.monitor(excludeTrailingPathDelimiter(spool));
/*      try {
        uint64_t timeout = server_.config_->value("spool_processing_interval",10000000u);
        sleepAsync(timeout);
      }
      catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_REQUEST_ABORTED + errorOffset ) throw;
#else
        if( e->code() != EINTR ) throw;
#endif
      }*/
    }
  }
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
