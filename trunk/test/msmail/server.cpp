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
Server::~Server()
{
}
//------------------------------------------------------------------------------
Server::Server(const ConfigSP config) :
  config_(config),
  rnd_(new Randomizer,rndMutex_),
  nodeClient_(NULL)
{
}
//------------------------------------------------------------------------------
void Server::open()
{
  stdErr.setDebugLevels(config_->value("debug_levels","+0,+1,+2,+3"));
//  maxThreads(1);
  ksock::Server::open();
  startNodeClient(stStandalone);
  uintptr_t i;
  for( i = config_->value("spool_fibers",1); i > 0; i-- )
    attachFiber(new SpoolWalker(*this));
  for( i = config_->value("mqueue_fibers",1); i > 0; i-- )
    attachFiber(new MailQueueWalker(*this));
}
//------------------------------------------------------------------------------
void Server::close()
{
  ksock::Server::close();
}
//------------------------------------------------------------------------------
Fiber * Server::newFiber()
{
  return new ServerFiber(*this);
}
//------------------------------------------------------------------------------
Server::Data & Server::data(ServerType type)
{
  switch( type ){
    case stNode       : return nodeData_;
    case stStandalone : return standaloneData_;
  }
  throw ExceptionSP(new Exception(EINVAL,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
utf8::String Server::spoolDir() const
{
  utf8::String spool(
    excludeTrailingPathDelimiter(
      config_->value("spool",getExecutablePath() + "spool")
    )
  );
  createDirectoryAsync(spool);
  return includeTrailingPathDelimiter(spool);
}
//------------------------------------------------------------------------------
utf8::String Server::mailDir() const
{
  utf8::String mail(
    excludeTrailingPathDelimiter(
      includeTrailingPathDelimiter(config_->value("spool",getExecutablePath() + "spool")) +
      "mail"
    )
  );
  createDirectoryAsync(mail);
  return includeTrailingPathDelimiter(mail);
}
//------------------------------------------------------------------------------
utf8::String Server::mqueueDir() const
{
  utf8::String mqueue(
    includeTrailingPathDelimiter(
      config_->value("spool",getExecutablePath() + "spool")
    ) +
    "mqueue"
  );
  createDirectoryAsync(mqueue);
  return includeTrailingPathDelimiter(mqueue);
}
//------------------------------------------------------------------------------
utf8::String Server::lckDir() const
{
  utf8::String lck(
    includeTrailingPathDelimiter(
      config_->value("spool",getExecutablePath() + "spool")
    ) +
    "lck"
  );
  createDirectoryAsync(lck);
  return includeTrailingPathDelimiter(lck);
}
//------------------------------------------------------------------------------
void Server::startNodeClient(ServerType dataType)
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  if( nodeClient_ == NULL ){
    NodeClient * nodeClient;
    attachFiber(nodeClient = new NodeClient(*this,dataType));
    nodeClient_ = nodeClient;
  }
}
//------------------------------------------------------------------------------
void Server::addRecvMailFiber(ServerFiber & fiber)
{
  AutoLock<FiberInterlockedMutex> lock(recvMailFibersMutex_);
  ServerFiber * fib = recvMailFibers_.find(fiber);
  if( fib != NULL ){
    fib->terminate();
    //abortNotification(&fib->dcn_);
    recvMailFibers_.remove(*fib);
  }
  recvMailFibers_.insert(fiber);
}
//------------------------------------------------------------------------------
void Server::remRecvMailFiber(ServerFiber & fiber)
{
  AutoLock<FiberInterlockedMutex> lock(recvMailFibersMutex_);
  ServerFiber * fib = recvMailFibers_.find(fiber);
  if( fib == &fiber ) recvMailFibers_.remove(fiber);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::Data::~Data()
{
}
//------------------------------------------------------------------------------
Server::Data::Data() : ftime_(0)
{
}
//------------------------------------------------------------------------------
bool Server::Data::registerUserNL(const UserInfo & info,uint64_t ftime)
{
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ) users_.insert(userList_.safeAdd(p = new UserInfo(info)));
    return true;
  }
  p->atime_ = gettimeofday();
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser(const UserInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUserNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKeyNL(const KeyInfo & info,uint64_t ftime)
{
  KeyInfo * p = keys_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ) keys_.insert(keyList_.safeAdd(p = new KeyInfo(info)));
    return true;
  }
  p->atime_ = gettimeofday();
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey(const KeyInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKeyNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroupNL(const GroupInfo & info,uint64_t ftime)
{
  GroupInfo * p = groups_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ) groups_.insert(groupList_.safeAdd(p = new GroupInfo(info)));
    return true;
  }
  p->atime_ = gettimeofday();
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroup(const GroupInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerGroupNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerServerNL(const ServerInfo & info,uint64_t ftime)
{
  ServerInfo * p = servers_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ) servers_.insert(serverList_.safeAdd(p = new ServerInfo(info)));
    return true;
  }
  p->atime_ = gettimeofday();
  if( info.node_ != p->node_ ){
    p->mtime_ = p->atime_;
    p->node_ = info.node_;
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerServer(const ServerInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerServerNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLinkNL(const User2KeyLink & link,uint64_t ftime)
{
  User2KeyLink * p = user2KeyLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ) user2KeyLinks_.insert(user2KeyLinkList_.safeAdd(p = new User2KeyLink(link)));
    return true;
  }
  p->atime_ = gettimeofday();
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLink(const User2KeyLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUser2KeyLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLinkNL(const Key2GroupLink & link,uint64_t ftime)
{
  Key2GroupLink * p = key2GroupLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ) key2GroupLinks_.insert(key2GroupLinkList_.safeAdd(p = new Key2GroupLink(link)));
    return true;
  }
  p->atime_ = gettimeofday();
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLink(const Key2GroupLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2GroupLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLinkNL(const Key2ServerLink & link,uint64_t ftime)
{
  Key2ServerLink * p = key2ServerLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ) key2ServerLinks_.insert(key2ServerLinkList_.safeAdd(p = new Key2ServerLink(link)));
    return true;
  }
  p->atime_ = gettimeofday();
  if( p->server_.strcasecmp(link.server_) != 0 ){
    p->mtime_ = p->atime_;
    p->server_ = link.server_;
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLink(const Key2ServerLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2ServerLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::intersectionNL(const Data & a,uint64_t ftime)
{
  bool r = false;
  intptr_t i;
  for( i = a.userList_.count() - 1; i >= 0; i-- )
    r = registerUserNL(a.userList_[i],ftime) || r;
  for( i = a.keyList_.count() - 1; i >= 0; i-- )
    r = registerKeyNL(a.keyList_[i],ftime) || r;
  for( i = a.groupList_.count() - 1; i >= 0; i-- )
    r = registerGroupNL(a.groupList_[i],ftime) || r;
  for( i = a.serverList_.count() - 1; i >= 0; i-- )
    r = registerServerNL(a.serverList_[i],ftime) || r;
  for( i = a.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    r = registerUser2KeyLinkNL(a.user2KeyLinkList_[i],ftime) || r;
  for( i = a.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2GroupLinkNL(a.key2GroupLinkList_[i],ftime_) || r;
  for( i = a.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2ServerLinkNL(a.key2ServerLinkList_[i],ftime) || r;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::intersection(const Data & a,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(a.mutex_);
  return intersectionNL(a,ftime);
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabaseNL(ksock::AsyncSocket & socket,uint64_t ftime)
{
  intptr_t i;
  uint64_t u;
  u = 0;
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].mtime_ > ftime ) socket << userList_[i];
  u = 0;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].mtime_ > ftime ) socket << keyList_[i];
  u = 0;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].mtime_ > ftime ) socket << groupList_[i];
  u = 0;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].mtime_ > ftime ) socket << serverList_[i];
  u = 0;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].mtime_ > ftime ) socket << user2KeyLinkList_[i];
  u = 0;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].mtime_ > ftime ) socket << key2GroupLinkList_[i];
  u = 0;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].mtime_ > ftime ) socket << key2ServerLinkList_[i];
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabase(ksock::AsyncSocket & socket,uint64_t ftime)
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  sendDatabaseNL(socket,ftime);
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabaseNL(ksock::AsyncSocket & socket,uint64_t ftime)
{
  union {
    intptr_t i;
    uint64_t u;
  };
  socket >> u;
  while( u-- > 0 ){
    UserInfo info;
    socket >> info;
    registerUserNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    KeyInfo info;
    socket >> info;
    registerKeyNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    GroupInfo info;
    socket >> info;
    registerGroupNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    ServerInfo info;
    socket >> info;
    registerServerNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    User2KeyLink link;
    socket >> link;
    registerUser2KeyLinkNL(link,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2GroupLink link;
    socket >> link;
    registerKey2GroupLinkNL(link,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2ServerLink link;
    socket >> link;
    registerKey2ServerLinkNL(link,ftime);
  }
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabase(ksock::AsyncSocket & socket,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  recvDatabaseNL(socket,ftime);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getNodeListNL() const
{
  utf8::String list;
  for( intptr_t i = serverList_.count() - 1; i >= 0; i-- ){
    if( !serverList_[i].node_ ) continue;
    if( list.strlen() > 0 ) list += ", ";
    list += serverList_[i].name_;
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getNodeList() const
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return getNodeListNL();
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
