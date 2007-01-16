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
Server::~Server()
{
}
//------------------------------------------------------------------------------
Server::Server(const ConfigSP config) :
  config_(config),
  rnd_(newObject<Randomizer>(),rndMutex_),
  nodeClient_(NULL),
  skippedNodeClientStarts_(0),
  skippedNodeExchangeStarts_(0),
  spoolFibers_(0)
{
}
//------------------------------------------------------------------------------
void Server::open()
{
  ksock::Server::open();
  spoolFibers_ = config_->valueByPath(utf8::String(serverConfSectionName[stStandalone]) + ".spool_fibers",8);
  mqueueCleanup();
  spoolCleanup();
  attachFiber(NodeClient::newClient(*this,stStandalone,utf8::String(),true));
  union {
    intptr_t i;
    uintptr_t u;
  };
  if( (bool) config_->valueByPath(utf8::String(serverConfSectionName[stStandalone]) + ".enabled",true) ){
    for( u = 1; u < spoolFibers_; u <<= 1 );
    spoolFibers_ = u;
    while( u > 0 ) attachFiber(newObjectV<SpoolWalker>(*this,--u));
  }
  else {
    i = 0;
  }
  attachFiber(newObjectV<SpoolWalker>(*this,--i)); // lost sheeps collector fiber and ctrl files handler
}
//------------------------------------------------------------------------------
void Server::close()
{
  closeSenders();
  ksock::Server::close();
}
//------------------------------------------------------------------------------
Fiber * Server::newFiber()
{
  return newObjectV<ServerFiber>(*this);
}
//------------------------------------------------------------------------------
utf8::String Server::spoolDirHelper() const
{
  return includeTrailingPathDelimiter(config_->valueByPath(
    utf8::String(serverConfSectionName[stStandalone]) + ".spool",
    getExecutablePath() + "spool"
  ));
}
//------------------------------------------------------------------------------
utf8::String Server::spoolDir(intptr_t id) const
{
  return includeTrailingPathDelimiter(spoolDirHelper() + (id >= 0 ? utf8::int2Str0(id,4) : utf8::String("collector")));
}
//------------------------------------------------------------------------------
utf8::String Server::mailDir() const
{
  return includeTrailingPathDelimiter(spoolDirHelper() + "mail");
}
//------------------------------------------------------------------------------
utf8::String Server::mqueueDir() const
{
  return includeTrailingPathDelimiter(spoolDirHelper() + "mqueue");
}
//------------------------------------------------------------------------------
utf8::String Server::lckDir() const
{
  return includeTrailingPathDelimiter(spoolDirHelper() + "lck");
}
//------------------------------------------------------------------------------
utf8::String Server::incompleteDir() const
{
  return includeTrailingPathDelimiter(spoolDirHelper() + "incomplete");
}
//------------------------------------------------------------------------------
void Server::clearNodeClient(NodeClient * client)
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  if( nodeClient_ == client ){
    nodeClient_ = NULL;
    if( skippedNodeClientStarts_ > 0 ){
      skippedNodeClientStarts_ = 0;
      startNodeClientNL(client->dataType_,client->nodeHostName_);
    }
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
}
//------------------------------------------------------------------------------
void Server::startNodeClientNL(ServerType dataType,const utf8::String & nodeHostName)
{
  if( shutdown_ ) return;
  if( nodeClient_ == NULL ){
    assert( skippedNodeClientStarts_ == 0 );
    NodeClient * nodeClient;
    attachFiber(nodeClient = NodeClient::newClient(*this,dataType,nodeHostName,false));
    nodeClient_ = nodeClient;
  }
  else {
    skippedNodeClientStarts_++;
  }
}
//------------------------------------------------------------------------------
void Server::startNodeClient(ServerType dataType,const utf8::String & nodeHostName)
{
  AutoLock<FiberInterlockedMutex> lock(nodeClientMutex_);
  startNodeClientNL(dataType,nodeHostName);
}
//------------------------------------------------------------------------------
void Server::startNodesExchangeNL()
{
  if( shutdown_ ) return;
  intptr_t i, j, c;
  utf8::String me(bindAddrs()[0].resolve(defaultPort));
  utf8::String hosts(data(stNode).getNodeList()), host;
  utf8::String neighbors = config_->parse().override().valueByPath(
    utf8::String(serverConfSectionName[stNode]) + ".neighbors",
    ""
  );
  if( hosts.strlen() > 0 ){
    if( neighbors.strlen() > 0 ) neighbors += ",";
    neighbors += hosts;
  }
  Array<utf8::String> nodes;
  for( i = enumStringParts(neighbors) - 1; i >= 0; i-- ){
    host = stringPartByNo(neighbors,i);
    ksock::SockAddr addr;
    addr.resolve(host,defaultPort);
    host = addr.resolve(defaultPort);
    if( host.strcasecmp(me) == 0 ) continue;
    j = nodes.bSearchCase(host,c);
    if( c != 0 ) nodes.insert(j + (c > 0),host);
  }
  if( nodeExchangeClients_.count() == 0 ){
    assert( skippedNodeExchangeStarts_ == 0 );
    try {
      for( j = nodes.count() - 1; j >= 0; j-- ){
        AutoPtr<NodeClient> nodeClient(NodeClient::newClient(*this,stNode,nodes[j],false));
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
  recvMailFibers_.insert(fiber,false,false,&fib);
  if( fib != &fiber ){
    fib->terminate();
    //abortNotification(&fib->dcn_);
    recvMailFibers_.remove(*fib);
    recvMailFibers_.insert(fiber);
  }
}
//------------------------------------------------------------------------------
bool Server::remRecvMailFiber(ServerFiber & fiber)
{
  bool r = false;
  AutoLock<FiberInterlockedMutex> lock(recvMailFibersMutex_);
  ServerFiber * fib = recvMailFibers_.find(fiber);
  if( fib == &fiber ){
    recvMailFibers_.remove(fiber);
    r = true;
  }
  return r;
}
//------------------------------------------------------------------------------
ServerFiber * Server::findRecvMailFiberNL(const ServerFiber & fiber)
{
  return recvMailFibers_.find(fiber);
}
//------------------------------------------------------------------------------
ServerFiber * Server::findRecvMailFiber(const ServerFiber & fiber)
{
  AutoLock<FiberInterlockedMutex> lock(recvMailFibersMutex_);
  return findRecvMailFiberNL(fiber);
}
//------------------------------------------------------------------------------
void Server::sendRobotMessage(
  const utf8::String & recepient,
  const utf8::String & sender,
  const utf8::String & sended,
  const utf8::String & key,
  const utf8::String & value,
  Message * msg)
{
  Message m;
  if( msg != NULL ) m.copyUserAttributes(*msg);
  m.value("#Recepient",recepient);
  m.value("#Sender",sender);
  utf8::String tms(getTimeString(gettimeofday()));
  m.value("#Sender.Sended",tms);
  m.value("#Sender.Sended.Origin",sended);
  m.value("#Sender.Process.Id",utf8::int2Str(ksys::getpid()));
  utf8::String psts(getTimeString(getProcessStartTime()));
  m.value("#Sender.Process.StartTime",psts);
  m.value("#Sender.Host","robot@" + bindAddrs()[0].resolve(defaultPort));
  m.value("#Relay.0.Process.Id",utf8::int2Str(ksys::getpid()));
  m.value("#Relay.0.Process.StartTime",psts);
  m.value("#Relay.0.Received",tms);
  m.value(key,value);
  AsyncFile file(incompleteDir() + m.id() + ".msg");
  file.createIfNotExist(true).removeAfterClose(true).open() << m;
  file.removeAfterClose(false).close();
  rename(file.fileName(),spoolDir(m.id().hash(true) & (spoolFibers_ - 1)) + m.id() + ".msg");
}
//------------------------------------------------------------------------------
void Server::sendUserWatchdog(const utf8::String & user)
{
// send notify to wait fiber
  AsyncFile watchdog(includeTrailingPathDelimiter(mailDir() + user) + "." + createGUIDAsBase32String());
  watchdog.createIfNotExist(true).removeAfterClose(true).open();
}
//------------------------------------------------------------------------------
void Server::sendMessage(const utf8::String & host,const utf8::String & id,const utf8::String & fileName)
{
  MailQueueWalker * pWalker, * pWalker2;
  AutoLock<FiberInterlockedMutex> lock(sendMailFibersMutex_);
  pWalker2 = newObjectV<MailQueueWalker>(*this,host);
  sendMailFibers_.insert(*pWalker2,false,true,&pWalker);
  if( pWalker == pWalker2 ) attachFiber(pWalker);
  AutoLock<FiberInterlockedMutex> lock2(pWalker->messagesMutex_);
  pWalker->messages_.insToTail(*newObject<Message::Key>(id));
  bool step0 = true;
  try {
    rename(fileName,mqueueDir() + id + ".msg");
  }
  catch( ExceptionSP & e ){
    pWalker->messages_.drop(*pWalker->messages_.last());
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->code() != ERROR_ALREADY_EXISTS + errorOffset ) throw;
#else
    if( e->code() != EEXIST ) throw;
#endif
    e->writeStdError();
    remove(fileName);
    step0 = false;
  }
  if( step0 ) if( pWalker->messages_.count() == 1 ) pWalker->semaphore_.post();
}
//------------------------------------------------------------------------------
void Server::removeSender(MailQueueWalker & sender) // must be called only from terminating fiber !!!
{
  AutoLock<FiberInterlockedMutex> lock(sendMailFibersMutex_);
  EmbeddedListNode<Message::Key> * p = sender.messages_.first();
  while( p != NULL ){
    utf8::String key(Message::Key::listNodeObject(*p));
    utf8::String fileName(mqueueDir() + key + ".msg");
    try {
      rename(
        fileName,
        spoolDir(key.hash(true) & (spoolFibers_ - 1)) + key + ".msg"
      );
    }
    catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_ALREADY_EXISTS + errorOffset ) throw;
#else
      if( e->code() != EEXIST ) throw;
#endif
      e->writeStdError();
      remove(fileName);
    }
    p = p->next();
  }
  sendMailFibers_.remove(sender);
}
//------------------------------------------------------------------------------
void Server::closeSenders()
{
  AutoLock<FiberInterlockedMutex> lock(sendMailFibersMutex_);
  Array<MailQueueWalker *> list;
  sendMailFibers_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    list[i]->terminate();
    list[i]->semaphore_.post();
  }
}
//------------------------------------------------------------------------------
void Server::mqueueCleanup()
{
  if( stat(excludeTrailingPathDelimiter(mqueueDir())) ){
    Vector<utf8::String> list;
    getDirList(list,mqueueDir() + "*.msg",utf8::String(),false);
    for( intptr_t i = list.count() - 1; i >= 0; i-- ){
      rename(
        list[i],
        spoolDir(getNameFromPathName(list[i]).hash(true) & (spoolFibers_ - 1)) +
          getNameFromPathName(list[i])
      );
      list.remove(i);
    }
  }
}
//------------------------------------------------------------------------------
void Server::spoolCleanup()
{
  for( uintptr_t i = spoolFibers_; i < ~uintptr_t(0); i++ ){
    if( !stat(excludeTrailingPathDelimiter(spoolDir(i))) ) break;
    Vector<utf8::String> list;
    getDirList(list,spoolDir(i) + "*.msg",utf8::String(),false);
    for( intptr_t j = list.count() - 1; j >= 0; j-- ){
      rename(
        list[j],
        spoolDir(getNameFromPathName(list[j]).hash(true) & (spoolFibers_ - 1)) +
          getNameFromPathName(list[j])
      );
      list.remove(j);
    }
    removeDirectory(excludeTrailingPathDelimiter(spoolDir(i)));
  }
}
//------------------------------------------------------------------------------
void Server::loadStaticDB()
{
  Server::Data & d = data(stStandalone);
  const ConfigSection & key2Server = config_->section("database").section("key2server");
  for( intptr_t i = key2Server.valueCount() - 1; i >= 0; i-- ){
    utf8::String key, value(key2Server.value(i,&key));
    d.registerKey2ServerLink(Key2ServerLink(unScreenString(key),unScreenString(value)));
  }
  const ConfigSection & key2Group = config_->section("database").section("key2group");
  for( intptr_t i = key2Group.valueCount() - 1; i >= 0; i-- ){
    utf8::String key, value(key2Group.value(i,&key));
    d.registerKey2GroupLink(Key2GroupLink(unScreenString(key),unScreenString(value)));
  }
}
//------------------------------------------------------------------------------
void Server::loadStaticRoutes()
{
  /*Server::Data & d = data(stStandalone);
  const ConfigSection & route = config_->section("route");
  for( intptr_t i = key2Server.valueCount() - 1; i >= 0; i-- ){
    utf8::String key, value(key2Server.value(i,&key));
    d.registerKey2ServerLink(Key2ServerLink(unScreenString(key),unScreenString(value)));
  }*/
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
