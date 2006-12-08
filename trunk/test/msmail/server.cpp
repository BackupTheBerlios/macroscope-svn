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
  mqueueCleanup();
  attachFiber(NodeClient::newClient(*this,stStandalone,utf8::String(),true));
  union {
    intptr_t i;
    uintptr_t u;
  };
  spoolFibers_ = config_->valueByPath(utf8::String(serverConfSectionName_[stStandalone]) + ".spool_fibers",8);
  for( u = 1; u < spoolFibers_; u <<= 1 );
  spoolFibers_ = u;
  while( u > 0 ) attachFiber(newObjectV<SpoolWalker>(*this,--u));
  attachFiber(newObjectV<SpoolWalker>(*this,--i)); // lost sheeps collector fiber  
//  for( i = config_->valueByPath(utf8::String(serverConfSectionName_[stStandalone]) + ".mqueue_fibers",8; i > 0; i-- )
//    attachFiber(newObjectV<MailQueueWalker>(*this));
}
//------------------------------------------------------------------------------
void Server::close()
{
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
    utf8::String(serverConfSectionName_[stStandalone]) + ".spool",
    getExecutablePath() + "spool"
  ));
}
//------------------------------------------------------------------------------
utf8::String Server::spoolDir(intptr_t id) const
{
  utf8::String spool(
    spoolDirHelper() + (id >= 0 ? utf8::int2Str0(id,4) : utf8::String("collector"))
  );
  createDirectory(spool);
  return includeTrailingPathDelimiter(spool);
}
//------------------------------------------------------------------------------
utf8::String Server::mailDir() const
{
  utf8::String mail(spoolDirHelper() + "mail");
  createDirectory(mail);
  return includeTrailingPathDelimiter(mail);
}
//------------------------------------------------------------------------------
utf8::String Server::mqueueDir() const
{
  utf8::String mqueue(spoolDirHelper() + "mqueue");
  createDirectory(mqueue);
  return includeTrailingPathDelimiter(mqueue);
}
//------------------------------------------------------------------------------
utf8::String Server::lckDir() const
{
  utf8::String lck(spoolDirHelper() + "lck");
  createDirectory(lck);
  return includeTrailingPathDelimiter(lck);
}
//------------------------------------------------------------------------------
utf8::String Server::incompleteDir() const
{
  utf8::String lck(spoolDirHelper() + "incomplete");
  createDirectory(lck);
  return includeTrailingPathDelimiter(lck);
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
    utf8::String(serverConfSectionName_[stNode]) + ".neighbors",
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
  if( fib != NULL ){
    fib->terminate();
    //abortNotification(&fib->dcn_);
    recvMailFibers_.remove(*fib);
  }
  recvMailFibers_.insert(fiber);
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
  MailQueueWalker * pWalker;
  AutoLock<FiberInterlockedMutex> lock(sendMailFibersMutex_);
  AutoPtr<Fiber> walker(pWalker = newObjectV<MailQueueWalker>(*this,host));
  if( sendMailFibers_.find(*pWalker) == NULL ) attachFiber(walker);
  AutoLock<FiberInterlockedMutex> lock2(pWalker->messagesMutex_);
  pWalker->messages_.insert(*newObject<Message::Key>(id));
  rename(fileName,mqueueDir() + id + ".msg");
  if( pWalker->messages_.count() == 1 ) pWalker->semaphore_.post();
}
//------------------------------------------------------------------------------
void Server::removeSender(MailQueueWalker & sender)
{
  AutoLock<FiberInterlockedMutex> lock(sendMailFibersMutex_);
  AutoLock<FiberInterlockedMutex> lock2(sender.messagesMutex_);
  Array<Message::Key *> list;
  sender.messages_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    rename(
      spoolDir(utf8::String(*list[i]).hash(true) & (spoolFibers_ - 1)) +
        *list[i] + ".msg",
      mqueueDir() + *list[i] + ".msg"
    );
  sendMailFibers_.remove(sender);
}
//------------------------------------------------------------------------------
void Server::mqueueCleanup()
{
  Vector<utf8::String> list;
  getDirList(list,mqueueDir() + "*.msg",utf8::String(),false);
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    rename(
      spoolDir(utf8::String(list[i]).hash(true) & (spoolFibers_ - 1)) +
        list[i] + ".msg",
      mqueueDir() + list[i] + ".msg"
    );
  }
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
