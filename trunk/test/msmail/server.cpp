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
Server::~Server()
{
}
//------------------------------------------------------------------------------
Server::Server(const ConfigSP config) :
  config_(config),
  rnd_(new Randomizer,rndMutex_),
  nodeClient_(NULL),
  skippedNodeClientStarts_(0),
  skippedNodeExchangeStarts_(0)
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
bool Server::clearNodeClient(NodeClient * client)
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  if( nodeClient_ == client ){
    if( skippedNodeClientStarts_ > 0 ){
      skippedNodeClientStarts_ = 0;
      return true;
    }
    nodeClient_ = NULL;
  }
  else {
    intptr_t i = nodeExchangeClients_.bSearch(client);
    assert( i >= 0 );
    nodeExchangeClients_.remove(i);
    if( nodeExchangeClients_.count() == 0 && skippedNodeExchangeStarts_ > 0 ){
      skippedNodeExchangeStarts_ = 0;
      startNodesExchangeNL();
    }
  }
  return false;
}
//------------------------------------------------------------------------------
void Server::startNodeClient(ServerType dataType,const utf8::String & nodeHostName)
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  if( nodeClient_ == NULL ){
    NodeClient * nodeClient;
    attachFiber(nodeClient = new NodeClient(*this,dataType,nodeHostName));
    nodeClient_ = nodeClient;
  }
  else {
    skippedNodeClientStarts_++;
  }
}
//------------------------------------------------------------------------------
void Server::startNodesExchangeNL()
{
  intptr_t i, j, c;
  utf8::String me(ksock::SockAddr::gethostname());
  utf8::String hosts(data(stNode).getNodeList()), host;
  if( nodeExchangeClients_.count() == 0 ){
    try {
      for( j = enumStringParts(hosts) - 1; j >= 0; j-- ){
        host = stringPartByNo(hosts,j);
        if( host.strcasecmp(me) == 0 ) continue;
        AutoPtr<NodeClient> nodeClient(new NodeClient(*this,stNode,host));
        i = nodeExchangeClients_.bSearch(nodeClient,c);
        assert( c != 0 );
        nodeExchangeClients_.insert(i += (c > 0),nodeClient);
        attachFiber(nodeExchangeClients_[i]);
        nodeClient.ptr(NULL);
      }
    }
    catch( ... ){
      nodeExchangeClients_.clear();
      throw;
    }
  }
  else {
    skippedNodeExchangeStarts_++;
  }
}
//------------------------------------------------------------------------------
void Server::startNodesExchange()
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  startNodesExchangeNL();
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
Server::Data::Data() : ftime_(0), stime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
/*Server::Data::Data(const Data & a)
{
  operator = (a);
}*/
//------------------------------------------------------------------------------
/*Server::Data & Server::Data::operator = (const Data & a)
{
  intptr_t i;
  ftime_ = a.ftime_;
  userList_ = a.userList_;
  for( i = userList_.count() - 1; i >= 0; i-- ) users_.insert(userList_[i]);
  keyList_ = a.keyList_;
  for( i = keyList_.count() - 1; i >= 0; i-- ) keys_.insert(keyList_[i]);
  groupList_ = a.groupList_;
  for( i = groupList_.count() - 1; i >= 0; i-- ) groups_.insert(groupList_[i]);
  serverList_ = a.serverList_;
  for( i = serverList_.count() - 1; i >= 0; i-- ) servers_.insert(serverList_[i]);
  user2KeyLinkList_ = a.user2KeyLinkList_;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- ) user2KeyLinks_.insert(user2KeyLinkList_[i]);
  key2GroupLinkList_ = a.key2GroupLinkList_;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- ) key2GroupLinks_.insert(key2GroupLinkList_[i]);
  key2ServerLinkList_ = a.key2ServerLinkList_;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- ) key2ServerLinks_.insert(key2ServerLinkList_[i]);
  return *this;
}*/
//------------------------------------------------------------------------------
bool Server::Data::registerUserNL(const UserInfo & info,uint64_t ftime)
{
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ){
      users_.insert(userList_.safeAdd(p = new UserInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
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
    if( info.mtime_ > ftime ){
      keys_.insert(keyList_.safeAdd(p = new KeyInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
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
    if( info.mtime_ > ftime ){
      groups_.insert(groupList_.safeAdd(p = new GroupInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
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
    if( info.mtime_ > ftime ){
      servers_.insert(serverList_.safeAdd(p = new ServerInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( info.mtime_ > ftime && info.type_ == stNode && info.type_ != p->type_ ){
      p->mtime_ = p->atime_;
      p->type_ = info.type_;
      return true;
    }
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
    if( link.mtime_ > ftime ){
      user2KeyLinks_.insert(user2KeyLinkList_.safeAdd(p = new User2KeyLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
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
    if( link.mtime_ > ftime ){
      key2GroupLinks_.insert(key2GroupLinkList_.safeAdd(p = new Key2GroupLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
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
    if( link.mtime_ > ftime ){
      key2ServerLinks_.insert(key2ServerLinkList_.safeAdd(p = new Key2ServerLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( link.mtime_ > ftime && p->server_.strcasecmp(link.server_) != 0 ){
      p->mtime_ = p->atime_;
      p->server_ = link.server_;
      return true;
    }
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
    if( !serverList_[i].type_ == stNode ) continue;
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
void Server::Data::dumpNL(utf8::String::Stream & stream) const
{
  intptr_t i;
  if( userList_.count() > 0 ){
    stream << "users:\n";
    for( i = userList_.count() - 1; i >= 0; i-- )
      stream << "  " << userList_[i] << "\n";
  }
  if( keyList_.count() > 0 ){
    stream << "keys:\n";
    for( i = keyList_.count() - 1; i >= 0; i-- )
      stream << "  " << keyList_[i] << "\n";
  }
  if( groupList_.count() > 0 ){
    stream << "groups:\n";
    for( i = groupList_.count() - 1; i >= 0; i-- )
      stream << "  " << groupList_[i] << "\n";
  }
  if( serverList_.count() > 0 ){
    stream << "servers:\n";
    for( i = serverList_.count() - 1; i >= 0; i-- )
      stream << "  " << serverList_[i] << "\n";
  }
  if( user2KeyLinkList_.count() > 0 ){
    stream << "user2key links:\n";
    for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << user2KeyLinkList_[i] << "\n";
  }
  if( key2GroupLinkList_.count() > 0 ){
    stream << "key2group links:\n";
    for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << key2GroupLinkList_[i] << "\n";
  }
  if( key2ServerLinkList_.count() > 0 ){
    stream << "key2server links:\n";
    for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << key2ServerLinkList_[i] << "\n";
  }
}
//------------------------------------------------------------------------------
void Server::Data::dump(utf8::String::Stream & stream) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  dumpNL(stream);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clear()
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  users_.clear();
  userList_.clear();
  keys_.clear();
  keyList_.clear();
  groups_.clear();
  groupList_.clear();
  servers_.clear();
  serverList_.clear();
  user2KeyLinks_.clear();
  user2KeyLinkList_.clear();
  key2GroupLinks_.clear();
  key2GroupLinkList_.clear();
  key2ServerLinks_.clear();
  key2ServerLinkList_.clear();
  ftime_ = 0;
  return *this;
}
//------------------------------------------------------------------------------
bool Server::Data::orNL(const Data & a,uint64_t ftime)
{
  intptr_t i;
  bool r = false;
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
    r = registerKey2GroupLinkNL(a.key2GroupLinkList_[i],ftime) || r;
  for( i = a.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2ServerLinkNL(a.key2ServerLinkList_[i],ftime) || r;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::or(const Data & a,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(a.mutex_);
  return orNL(a,ftime);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xorNL(const Data & data1,const Data & data2,uint64_t ftime)
{
  intptr_t i;
  for( i = data2.userList_.count() - 1; i >= 0; i-- )
    if( data1.users_.find(data2.userList_[i]) == NULL )
      registerUserNL(data2.userList_[i],ftime);
  for( i = data2.keyList_.count() - 1; i >= 0; i-- )
    if( data1.keys_.find(data2.keyList_[i]) == NULL )
      registerKeyNL(data2.keyList_[i],ftime);
  for( i = data2.groupList_.count() - 1; i >= 0; i-- )
    if( data1.groups_.find(data2.groupList_[i]) == NULL )
      registerGroupNL(data2.groupList_[i],ftime);
  for( i = data2.serverList_.count() - 1; i >= 0; i-- )
    if( data1.servers_.find(data2.serverList_[i]) == NULL )
      registerServerNL(data2.serverList_[i],ftime);
  for( i = data2.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( data1.user2KeyLinks_.find(data2.user2KeyLinkList_[i]) == NULL )
      registerUser2KeyLinkNL(data2.user2KeyLinkList_[i],ftime);
  for( i = data2.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2GroupLinks_.find(data2.key2GroupLinkList_[i]) == NULL )
      registerKey2GroupLinkNL(data2.key2GroupLinkList_[i],ftime);
  for( i = data2.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2ServerLinks_.find(data2.key2ServerLinkList_[i]) == NULL )
      registerKey2ServerLinkNL(data2.key2ServerLinkList_[i],ftime);
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xor(const Data & data1,const Data & data2,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(data1.mutex_);
  AutoMutexRDLock<FiberMutex> lock2(data2.mutex_);
  return xorNL(data1,data2,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::sweepNL(uint64_t stime,utf8::String::Stream * log)
{
  if( stime_ >= stime ) return false;
  intptr_t i;
  bool r = false;
  for( i = userList_.count() - 1; i >= 0; i-- ){
    if( userList_[i].atime_ < stime ){
      if( log != NULL ) *log << userList_[i] << "\n";
      users_.remove(userList_[i]);
      userList_.remove(i);
      r = true;
    }
  }
  for( i = keyList_.count() - 1; i >= 0; i-- ){
    if( keyList_[i].atime_ < stime ){
      if( log != NULL ) *log << keyList_[i] << "\n";
      keys_.remove(keyList_[i]);
      keyList_.remove(i);
      r = true;
    }
  }
  for( i = groupList_.count() - 1; i >= 0; i-- ){
    if( groupList_[i].atime_ < stime ){
      if( log != NULL ) *log << groupList_[i] << "\n";
      groups_.remove(groupList_[i]);
      groupList_.remove(i);
      r = true;
    }
  }
  for( i = serverList_.count() - 1; i >= 0; i-- ){
    if( serverList_[i].atime_ < stime ){
      if( log != NULL ) *log << serverList_[i] << "\n";
      servers_.remove(serverList_[i]);
      serverList_.remove(i);
      r = true;
    }
  }
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- ){
    if( user2KeyLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << user2KeyLinkList_[i] << "\n";
      user2KeyLinks_.remove(user2KeyLinkList_[i]);
      user2KeyLinkList_.remove(i);
      r = true;
    }
  }
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- ){
    if( key2GroupLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << key2GroupLinkList_[i] << "\n";
      key2GroupLinks_.remove(key2GroupLinkList_[i]);
      key2GroupLinkList_.remove(i);
      r = true;
    }
  }
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- ){
    if( key2ServerLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << key2ServerLinkList_[i] << "\n";
      key2ServerLinks_.remove(key2ServerLinkList_[i]);
      key2ServerLinkList_.remove(i);
      r = true;
    }
  }
  stime_ = stime;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::sweep(uint64_t stime,utf8::String::Stream * log)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return sweepNL(stime,log);
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
