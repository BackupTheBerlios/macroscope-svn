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
//  maxThreads(1);
  ksock::Server::open();
  startNodesExchange();
  attachFiber(new NodeClient(*this,stStandalone,utf8::String(),true));
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
utf8::String Server::incompleteDir() const
{
  utf8::String lck(
    includeTrailingPathDelimiter(
      config_->value("spool",getExecutablePath() + "spool")
    ) +
    "incomplete"
  );
  createDirectoryAsync(lck);
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
  if( nodeClient_ == NULL ){
    assert( skippedNodeClientStarts_ == 0 );
    NodeClient * nodeClient;
    attachFiber(nodeClient = new NodeClient(*this,dataType,nodeHostName,false));
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
  intptr_t i, j, c;
  utf8::String me(ksock::SockAddr::gethostname());
  utf8::String hosts(data(stNode).getNodeList()), host;
  if( hosts.strlen() > 0 ) hosts += ",";
  hosts += config_->parse().override().valueByPath("node.neighbors","");
  if( nodeExchangeClients_.count() == 0 ){
    assert( skippedNodeExchangeStarts_ == 0 );
    try {
      for( j = enumStringParts(hosts) - 1; j >= 0; j-- ){
        host = stringPartByNo(hosts,j);
        if( host.strcasecmp(me) == 0 ) continue;
        AutoPtr<NodeClient> nodeClient(new NodeClient(*this,stNode,host,false));
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
} // namespace msmail
//------------------------------------------------------------------------------
